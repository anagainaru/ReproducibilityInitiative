#include <iostream>
#include <vector>
#include <random>    
#include <algorithm> 
#include <functional>
#include <chrono>

#include <adios2.h>
#include <mpi.h>

std::vector<float> create_random_data(int n) {
    std::random_device r;
    std::seed_seq      seed{r(), r(), r(), r(), r(), r(), r(), r()};
    std::mt19937       eng(seed);

    std::uniform_int_distribution<int> dist;
    std::vector<float> v(n);

    generate(begin(v), end(v), bind(dist, eng));
    return v;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0] << "runid array_size number_variables"
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
    int mpiGroup = 0;
    MPI_Comm_split(MPI_COMM_WORLD, mpiGroup, rank, &mpiComm);
    MPI_Comm_rank(mpiComm, &rank);
    MPI_Comm_size(mpiComm, &size);
    auto myFloats = create_random_data(Nx);

    try
    {
        adios2::ADIOS adios(mpiComm);
        adios2::IO bpIO = adios.DeclareIO("myIO" + std::to_string(runid));
        bpIO.SetEngine("Bp");

        std::vector<adios2::Variable<float>> bpFloats(variablesSize);
        for (unsigned int v = 0; v < variablesSize; ++v)
        {
            std::string namev("bpFloats");
            namev += std::to_string(v);
            bpFloats[v] = bpIO.DefineVariable<float>(namev, {size * Nx},
                                                      {rank * Nx}, {Nx});
        }

        // Create engine smart pointer to Bp Engine due to polymorphism,
        // Open returns a smart pointer to Engine containing the Derived class
        adios2::Engine bpWriter = bpIO.Open("helloBp" + std::to_string(runid),
					      adios2::Mode::Write);
        double put_time = 0;
        for (unsigned int timeStep = 0; timeStep < 1; ++timeStep)
        {
            auto start_step = std::chrono::steady_clock::now();
            bpWriter.BeginStep();
            for (unsigned int v = 0; v < variablesSize; ++v)
            {
                myFloats[rank] += static_cast<float>(v + rank);
                auto start_put = std::chrono::steady_clock::now();
                bpWriter.Put<float>(bpFloats[v], myFloats.data());
                auto end_put = std::chrono::steady_clock::now();
                put_time += (end_put - start_put).count() / 1000;
            }
            bpWriter.EndStep();
            auto end_step = std::chrono::steady_clock::now();
	    double total_time = (end_step - start_step).count() / 1000;

	    double global_put_sum = 0;
	    MPI_Reduce(&put_time, &global_put_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
		   mpiComm);
	    double global_sum = 0;
	    MPI_Reduce(&total_time, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
		   mpiComm);

	    // Time in microseconds
	    if (rank == 0)
		std::cout << "BP,Write," << size << "," << Nx << ","
			  << variablesSize << "," << global_put_sum / size << ","
			  << global_sum / size << std::endl;
        }
        bpWriter.Close();
    }
    catch (std::invalid_argument &e)
    {
        std::cout << "Invalid argument exception, STOPPING PROGRAM from rank "
                  << rank << "\n";
        std::cout << e.what() << "\n";
    }
    catch (std::ios_base::failure &e)
    {
        std::cout
            << "IO System base failure exception, STOPPING PROGRAM from rank "
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
