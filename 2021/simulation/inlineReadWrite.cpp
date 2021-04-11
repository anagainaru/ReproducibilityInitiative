#include <algorithm> //std::for_each
#include <ios>       //std::ios_base::failure
#include <iostream>  //std::cout
#include <stdexcept> //std::invalid_argument std::exception
#include <vector>
#include <chrono>
#include <random>
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
        int rank;
        MPI_Comm_rank(mpiComm, &rank);
        MPI_Gather(myFloats.data(), Nx, MPI_FLOAT, allData.data(), Nx,
                      MPI_FLOAT, 0, mpiComm);
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

double read_simulation(adios2::IO &inlineIO, adios2::Engine &inlineReader, int rank,
                 int size, size_t variablesSize, unsigned int step,
                 int analysis_amount, int analysis_type, double *analysis_time)
{
    int Nx = 0;
    std::vector<std::vector<float>> myFloats(variablesSize);
    auto start_step = std::chrono::steady_clock::now();
    inlineReader.BeginStep();
    // READ
    for (unsigned int v = 0; v < variablesSize; ++v)
    {
        std::string namev("inlineFloats");
        namev += std::to_string(v);
        adios2::Variable<float> inlineFloats =
            inlineIO.InquireVariable<float>(namev);

        if (inlineFloats)
        {
            Nx = (inlineFloats.Shape()[0] / size);
            auto blocksInfo = inlineReader.BlocksInfo(inlineFloats, step);

            for (auto &info : blocksInfo)
            {
                // bp file reader would see all blocks, inline only sees local
                // writer's block(s).
                size_t myBlock = info.BlockID;
                inlineFloats.SetBlockSelection(myBlock);

                inlineReader.Get<float>(inlineFloats, info,
                                        adios2::Mode::Deferred);
            }
            inlineReader.PerformGets();
            
            for (const auto &info : blocksInfo)
            {
                adios2::Dims count = info.Count;
                myFloats[v].resize(count[0]);
                myFloats[v].assign(info.Data(), info.Data() + count[0]);
            }
        }
        else
        {
            std::cout << "Variable inlineFloats not found\n";
        }
    }

    inlineReader.EndStep();
    double time = DoAnalysis(analysis_amount, analysis_type, Nx,
            myFloats[0], size, MPI_COMM_WORLD);
    (* analysis_time) += time;

    auto end_step = std::chrono::steady_clock::now();
    double total_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_step - start_step).count();
    return total_time;
}

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        std::cout << "Usage: " << argv[0] << " runid array_size computation_amount "
	        	  << "analysis_amount simulation_steps" << std::endl;
        return -1;
    }
    int runid = atoi(argv[1]);
    const size_t Nx = atoi(argv[2]);
    const size_t variablesSize = 1;
    const int simulation_steps = atoi(argv[5]);
    const int computation_amount = atoi(argv[3]);
    const int analysis_amount = atoi(argv[4]);
    const int analysis_type = 1;

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Application variable
    auto myFloats = create_random_data(Nx);

    try
    {
        // Inline uses single IO for write/read
        adios2::ADIOS adios(MPI_COMM_WORLD);
        adios2::IO inlineIO = adios.DeclareIO(
		"InlineReadWrite" + std::to_string(runid));
        // WRITE
        inlineIO.SetEngine("Inline");

        std::vector<adios2::Variable<float>> inlineFloats(variablesSize);
        for (unsigned int v = 0; v < variablesSize; ++v)
        {
            std::string namev("inlineFloats");
            namev += std::to_string(v);
            inlineFloats[v] = inlineIO.DefineVariable<float>(
                namev, {size * Nx}, {rank * Nx}, {Nx}, adios2::ConstantDims);
        }

        adios2::Engine inlineWriter =
            inlineIO.Open("myWriteID" + std::to_string(runid),
	                  adios2::Mode::Write);

        adios2::Engine inlineReader =
            inlineIO.Open("myReadID" + std::to_string(runid),
                          adios2::Mode::Read);

        double write_time = 0, read_time = 0, analysis_time = 0;
        for (unsigned int timeStep = 0; timeStep < simulation_steps; ++timeStep)
        {
            auto start_step = std::chrono::steady_clock::now();
            // do computation
            sleep(computation_amount);
            // stream data
            inlineWriter.BeginStep();
            for (unsigned int v = 0; v < variablesSize; ++v)
            {
                myFloats[rank] += static_cast<float>(v + rank);
                inlineWriter.Put(inlineFloats[v], myFloats.data());
            }
            inlineWriter.EndStep();
            auto end_step = std::chrono::steady_clock::now();
            write_time += std::chrono::duration_cast<std::chrono::microseconds>(
                    end_step - start_step).count();

             read_time += read_simulation(inlineIO, inlineReader, rank, size,
                     variablesSize, timeStep, analysis_amount, analysis_type,
                     &analysis_time);
        }
        double global_write;
        MPI_Reduce(&write_time, &global_write, 1, MPI_DOUBLE, MPI_SUM, 0,
                MPI_COMM_WORLD);

        double global_read;
        MPI_Reduce(&read_time, &global_read, 1, MPI_DOUBLE, MPI_SUM, 0,
                MPI_COMM_WORLD);
        double global_analysis;
        MPI_Reduce(&analysis_time, &global_analysis, 1, MPI_DOUBLE, MPI_SUM, 0,
                MPI_COMM_WORLD);

        // Time in microseconds
        if (rank == 0)
        {
            double computation_time_micros = computation_amount * simulation_steps;
            computation_time_micros *= 1000000;
            std::cout << "Inline,Write," << size << "," << Nx << ","
                      << variablesSize << "," << simulation_steps << ","
                      << computation_time_micros << "," 
                      << global_write / size << std::endl;
            std::cout << "Inline,Read," << size << "," << Nx << ","
                      << variablesSize << "," << simulation_steps << ","
                      << global_analysis / size << "," << global_read / size
                      << std::endl;
        }
    }
    catch (std::exception const &e)
    {
        std::cout << "Caught exception from rank " << rank << "\n";
        std::cout << e.what() << "\n";
        return MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Finalize();
    return 0;
}
