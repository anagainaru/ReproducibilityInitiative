#include <chrono>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

#include <adios2.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0] << "id array_size number_variables"
                  << std::endl;
        return -1;
    }
    int runid = atoi(argv[1]);
    const size_t Nx = atoi(argv[2]);
    const size_t variablesSize = atoi(argv[3]);

    int rank = 0, size = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm mpiComm;
    // if executed in MPMD mode size will include both readers and writers
    int mpiGroup = 1;
    MPI_Comm_split(MPI_COMM_WORLD, mpiGroup, rank, &mpiComm);
    MPI_Comm_rank(mpiComm, &rank);
    MPI_Comm_size(mpiComm, &size);

    std::vector<float> myFloats;

    try
    {
        adios2::ADIOS adios(mpiComm);

        adios2::IO bpIO = adios.DeclareIO("myIO" + std::to_string(runid));
        bpIO.SetEngine("Bp");

        adios2::Engine bpReader = bpIO.Open("helloBp" + std::to_string(runid),
					      adios2::Mode::Read);

        double get_time = 0;
        const std::size_t my_start = Nx * rank;
        const adios2::Dims pos_start{my_start};
        const adios2::Dims count{Nx};
        const adios2::Box<adios2::Dims> sel(pos_start, count);
        myFloats.resize(Nx);

        auto start_step = std::chrono::steady_clock::now();
        bpReader.BeginStep();
        for (unsigned int v = 0; v < variablesSize; ++v)
        {
            std::string namev("bpFloats");
            namev += std::to_string(v);
            adios2::Variable<float> bpFloats =
                bpIO.InquireVariable<float>(namev);

            bpFloats.SetSelection(sel);
            auto start_get = std::chrono::steady_clock::now();
            bpReader.Get(bpFloats, myFloats.data());
            auto end_get = std::chrono::steady_clock::now();
            get_time += (end_get - start_get).count() / 1000;
        }
        bpReader.EndStep();
        auto end_step = std::chrono::steady_clock::now();
	double total_time = (end_step - start_step).count() / (size * 1000);
	get_time /= size;

	double global_get_sum = 0;
	MPI_Reduce(&get_time, &global_get_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
		   mpiComm);
	double global_sum = 0;
	MPI_Reduce(&total_time, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
		   mpiComm);

	// Time in microseconds
	if (rank == 0){
		std::cout << "BP,Read," << size << "," << Nx << ","
			  << variablesSize << "," << global_get_sum << ","
			  << global_sum  << std::endl;
	    }
        bpReader.Close();
    }
    catch (std::invalid_argument &e)
    {
        std::cout << "Invalid argument exception, STOPPING PROGRAM from rank "
                  << rank << "\n";
        std::cout << e.what() << "\n";
    }
    catch (std::ios_base::failure &e)
    {
        std::cout << "IO System base failure exception, STOPPING PROGRAM "
                     "from rank "
                  << rank << "\n";
        std::cout << e.what() << "\n";
    }
    catch (std::exception &e)
    {
        std::cout << "Exception, STOPPING PROGRAM from rank " << rank << "\n";
        std::cout << e.what() << "\n";
    }

    MPI_Finalize();
    return 0;
}
