#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <unistd.h>

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

double DoAnalysis(int analysis_amount, int analysis_type, int Nx,
                std::vector<float> &myFloats, int size, MPI_Comm mpiComm)
{
    auto start_sim = std::chrono::steady_clock::now();
    sleep(analysis_amount);
    if (analysis_type == 1)
    {
        // gather all the data and split it in a different way
        // then scatter it back to every process
        std::vector<float> allData(Nx * size);
        MPI_Gather(allData.data(), Nx, MPI_FLOAT, myFloats.data(), Nx,
                      MPI_FLOAT, 0, mpiComm);
        int rank;
        MPI_Comm_rank(mpiComm, &rank);
        if (rank == 0)
            std::rotate(allData.begin(), allData.begin() + 1, allData.end());
        MPI_Scatter(allData.data(), Nx, MPI_FLOAT, myFloats.data(),
                    Nx, MPI_FLOAT, 0, mpiComm);
    }
    auto end_sim = std::chrono::steady_clock::now();
    double total_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_sim - start_sim).count();
    return total_time;
}

void read_simulation(int runid, adios2::IO sscIO, int rank, int size, size_t Nx_write,
                     size_t variablesSize, MPI_Comm mpiComm, int number_writers,
                     int simulation_steps, int analysis_amount, int analysis_type)
{
    size_t Nx = Nx_write * number_writers / size;
    const std::size_t my_start = Nx * rank;
    const adios2::Dims start{my_start};
    const adios2::Dims count{Nx};
    const adios2::Box<adios2::Dims> sel(start, count);

    std::vector<std::vector<float>> myFloats(variablesSize);
    for (unsigned int v = 0; v < variablesSize; v++)
    {
        myFloats[v].resize(Nx);
    }
    adios2::Engine sscReader = sscIO.Open("helloSsc" + std::to_string(runid),
	   			          adios2::Mode::Read);
    
    // Start the analysis
    double analysis_time = 0;
    auto start_sim = std::chrono::steady_clock::now();
    sscReader.LockReaderSelections();
    for (int step = 0; step < simulation_steps; step++)
    {
        sscReader.BeginStep();
        for (unsigned int v = 0; v < variablesSize; v++)
        {
            std::string namev("sscFloats");
            namev += std::to_string(v);
            adios2::Variable<float> sscFloats =
                sscIO.InquireVariable<float>(namev);

            sscFloats.SetSelection(sel);
            sscReader.Get(sscFloats, myFloats[v].data());
        }
        sscReader.EndStep();
        analysis_time += DoAnalysis(analysis_amount, analysis_type, Nx,
                                    myFloats[0], size, mpiComm);
    }
    auto end_sim = std::chrono::steady_clock::now();
    double total_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_sim - start_sim).count();

    double global_sum = 0;
    MPI_Reduce(&total_time, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
               mpiComm);
    double global_analysis = 0;
    MPI_Reduce(&analysis_time, &global_analysis, 1, MPI_DOUBLE, MPI_SUM, 0,
               mpiComm);

    // Time in microseconds
    if (rank == 0){
        std::cout << "SSC,Read," << size << "," << Nx << ","
                  << variablesSize << "," << simulation_steps << ","
                  << analysis_time / size << "," << global_sum / size
                  << std::endl;
    }
    sscReader.Close();
}

void write_simulation(int runid, adios2::IO sscIO, int rank, int size,
                      size_t Nx, size_t variablesSize, MPI_Comm mpiComm,
                      int simulation_steps, int computation_amount)
{
    // Application variable
    auto myFloats = create_random_data(Nx);
    adios2::Engine sscWriter = sscIO.Open("helloSsc" + std::to_string(runid),
	   		 	          adios2::Mode::Write);

    // Define ADIOS variables and local size
    std::vector<adios2::Variable<float>> sscFloats(variablesSize);
    for (unsigned int v = 0; v < variablesSize; ++v)
    {
        std::string namev("sscFloats");
        namev += std::to_string(v);
        sscFloats[v] = sscIO.DefineVariable<float>(
            namev, {size * Nx}, {rank * Nx}, {Nx});
    }

    auto start_sim = std::chrono::steady_clock::now();
    sscWriter.LockWriterDefinitions();
    for(int step = 0; step < simulation_steps; step++)
    {
        // do computation
        sleep(computation_amount);
        // stream data
        sscWriter.BeginStep();
        for (unsigned int v = 0; v < variablesSize; v++)
        {
            myFloats[rank] += static_cast<float>(rank + step - v);
            sscWriter.Put<float>(sscFloats[v], myFloats.data());
        }
        sscWriter.EndStep();
    }
    auto end_sim = std::chrono::steady_clock::now();
    double total_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_sim - start_sim).count();

    double global_sum;
    MPI_Reduce(&total_time, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
           mpiComm);

    // Time in microseconds
    if (rank == 0)
    {
        double computation_time_micros = computation_amount * simulation_steps;
        computation_time_micros *= 1000000;
        std::cout << "SSC,Write," << size << "," << Nx << ","
                  << variablesSize << "," << simulation_steps << ","
                  << computation_time_micros << "," 
                  << global_sum / size << std::endl;
    }
    sscWriter.Close();
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm mpiComm;
    int worldRank, worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    int rank, size, number_readers = worldSize / 2;

    if (argc < 6)
    {
        std::cout << "Usage: " << argv[0] << "runid array_size computation_amount "
	        	  << "analysis_amount simulation_steps [readers]" << std::endl;
        return -1;
    }
    int runid = atoi(argv[1]);
    const size_t Nx = atoi(argv[2]);
    const size_t variablesSize = 1;
    const int simulation_steps = atoi(argv[5]);
    const int computation_amount = atoi(argv[3]);
    const int analysis_amount = atoi(argv[4]);
    const int analysis_type = 0;

    if (argc == 7)
	    number_readers = atoi(argv[6]);

    int mpiGroup = 1, number_writers = worldSize - number_readers;
    if (number_writers <= 0 || number_readers <=0){
        std::cout << "Invalide number of readers / writers" << std::endl;
        MPI_Finalize();
        return 1;
    }
    if (worldRank >= number_readers) mpiGroup = 0;
    MPI_Comm_split(MPI_COMM_WORLD, mpiGroup, worldRank, &mpiComm);
    MPI_Comm_rank(mpiComm, &rank);
    MPI_Comm_size(mpiComm, &size);

    // Simulation starts here
    try
    {
        adios2::ADIOS adios(mpiComm);
        adios2::IO sscIO = adios.DeclareIO("myIO" + std::to_string(runid));
        sscIO.SetEngine("Ssc");
	
	    // each writer process puts variableSize * Nx floats to the stream
        if (mpiGroup==0)
            write_simulation(runid, sscIO, rank, size,
                       Nx, variablesSize, mpiComm, simulation_steps,
                       computation_amount);
	    // each reader process gets (varSize * Nx * num_writers) / num_readers floats
        if (mpiGroup == 1)
            read_simulation(runid, sscIO, rank, size, Nx, variablesSize,
		              mpiComm, number_writers, simulation_steps, analysis_amount,
                      analysis_type);
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
