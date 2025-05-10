#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <filesystem>

#include <swarm_intelligence/common/instance_reader.hpp>

#include <swarm_intelligence/ACO/solver.hpp>
#include <swarm_intelligence/ACO/util.hpp>

#include <swarm_intelligence/PSO/solver.hpp>
#include <swarm_intelligence/PSO/util.hpp>

namespace fs = std::filesystem;

struct TSolution {
    int64_t RouteCnt;
    int64_t BestDistance;
};

struct TStat {
    std::string Name;
    std::string Description;
    TSolution Best;
    TSolution Mine;
    int64_t Time; // in milliseconds

    double FindPercent() const {
        double one = static_cast<double>(Best.BestDistance) / 100.;
        return (static_cast<double>(Mine.BestDistance) / one) - 100.;
    }
};

// std::pair<SI::CVRP::TFileInstanceReader, SI::CVRP::TProblem> ReadInstance(const std::string& filename) {
//     SI::CVRP::TFileInstanceReader reader(filename);
//     return {reader, reader.Read()};
// }

std::pair<int64_t, int64_t> ReadBestSolution(std::string filename) {
    filename.pop_back(); filename.pop_back(); filename.pop_back(); // vrp
    filename += "sol";
    std::ifstream file(filename);
    std::string line;

    int64_t routeCnt = 0;
    int64_t bestDistance = 0;
    while (std::getline(file, line)) {
        if (auto pos = line.find("Cost "); pos != std::string::npos) {
            bestDistance = std::stoll(line.substr(pos + 5));
            break;
        }
        routeCnt++;
    }

    file.close();

    return {routeCnt, bestDistance};
}

SI::CVRP::TSolution RunACO(const SI::CVRP::TProblem& problem, const SI::ACO::TParameters& parameters) {
    SI::ACO::TSolver solverACO(problem, parameters);
    SI::CVRP::TSolution solutionACO = solverACO.Solve();
    return solutionACO;
}

SI::CVRP::TSolution RunPSO(const SI::CVRP::TProblem& problem, const SI::PSO::TParameters& parameters) {
    SI::PSO::TSolver solverPSO(problem, parameters);
    SI::CVRP::TSolution solutionPSO = solverPSO.Solve();
    return solutionPSO;
}

void PrintRoutes(const SI::CVRP::TSolution& solution) {
    int i = 0;
    for (const auto& route : solution.Routes()) {
        std::cout << "Route " << ++i << ": ";
        for (const auto& node : solution.RouteWithCoordinates(route)) {
            if (node.ID == 0) {
                continue;
            }
            std::cout << node.ID + 1 << " ";
        }
        std::cout << std::endl;
    }
}

template<typename T>
struct TParameterIterator {
    T& Parameter;
    T Delta;
    T BeginNum;
    T EndNum;
    T Best;
    double BestPercent = 0;

    TParameterIterator(T& parameter, T delta, T beginNum, T endNum)
        : Parameter(parameter)
        , Delta(delta)
        , BeginNum(beginNum)
        , EndNum(endNum)
        , Best(BeginNum)
    {
        Parameter = BeginNum;
    }

    TParameterIterator& operator++() {
        Parameter += Delta;
        Parameter = std::min(Parameter, EndNum);
        return *this;
    }

    TParameterIterator& operator--() {
        Parameter -= Delta;
        Parameter = std::max(Parameter, BeginNum);
        return *this;
    }

    bool end() const {
        return Parameter == EndNum;
    }

    bool begin() const {
        return Parameter == BeginNum;
    }

    T& operator*() const {
        return Parameter;
    }

    void CheckBest(double percent) {
        if (percent < BestPercent) {
            Best = Parameter;
        }
    }
};

template<typename T>
struct TBestSearcher {
    T BestParameters;
    double BestPercent = 0;

    void CheckBest(double percent, const T& parameters) {
        if (percent < BestPercent) {
            BestParameters = parameters;
        }
    }
};

void SearchBestPso(const std::string& path) {
    SI::PSO::TParameters psoParameters;
    TBestSearcher<SI::PSO::TParameters> bestSearcher;

    for (auto iterW = TParameterIterator<double>(psoParameters.W, 0.05, 0.05, 1); !iterW.end(); ++iterW) {
        for (auto iterC1 = TParameterIterator<double>(psoParameters.C1, 0.05, 0.05, 1); !iterC1.end(); ++iterC1) {
            for (auto iterC2 = TParameterIterator<double>(psoParameters.C2, 0.05, 0.05, 1); !iterC2.end(); ++iterC2) {
                if (iterC1.Parameter + iterC2.Parameter + iterW.Parameter - 0.000001 > 1. || iterC1.Parameter + iterC2.Parameter + iterW.Parameter + 0.000001 < 1.) {
                    continue;
                }
                for (auto iterM = TParameterIterator<double>(psoParameters.M, 0.05, 0.05, 1); !iterM.end(); ++iterM) {
                    for (auto iterMaxIter = TParameterIterator<int64_t>(psoParameters.MaxIterationsWitoutImprovement, 1, 100, 1); !iterMaxIter.end(); ++iterMaxIter) {
                        std::vector<TStat> statsPSO;
                        for (const auto& entry : fs::directory_iterator(path)) {
                            std::string filename = entry.path().filename();
                            if (filename.find(".vrp") == std::string::npos) {
                                continue;
                            }
                            filename = path + filename;

                            // auto [reader, problem] = ReadInstance(filename);
                            SI::CVRP::TFileInstanceReader reader(filename);
                            auto [bestCntRoutes, bestDistance] = ReadBestSolution(filename);
                            SI::CVRP::TProblem problem = reader.Read();

                            psoParameters.SwarmSize = std::max(static_cast<int64_t>(100), reader.Dimension);

                            SI::CVRP::TSolution solutionPSO = RunPSO(problem, psoParameters);

                            // std::cout << "PSO solution: " << solutionPSO.TotalDistance() / SI::CVRP::TFileInstanceReader::MULTIPLIER
                            // << " total distance and " << solutionPSO.Routes().size() << " routes" << std::endl;

                            statsPSO.emplace_back(TStat{
                                reader.Name,
                                reader.Comment,
                                {bestCntRoutes, bestDistance},
                                {solutionPSO.Routes().size(), solutionPSO.TotalDistance() / SI::CVRP::TFileInstanceReader::MULTIPLIER},
                            });
                        }

                        double percentPSO = 0;
                        for (size_t i = 0; i < statsPSO.size(); i++) {
                            percentPSO = (percentPSO + statsPSO[i].FindPercent()) / 2;
                        }

                        bestSearcher.CheckBest(percentPSO, psoParameters);
                    }
                }
            }
        }
    }

    std::cout << "Best parameters:" << std::endl;
    std::cout << "W: " << bestSearcher.BestParameters.W << std::endl;
    std::cout << "C1: " << bestSearcher.BestParameters.C1 << std::endl;
    std::cout << "C2: " << bestSearcher.BestParameters.C2 << std::endl;
    std::cout << "M: " << bestSearcher.BestParameters.M << std::endl;
    std::cout << "MaxIterationsWitoutImprovement: " << bestSearcher.BestParameters.MaxIterationsWitoutImprovement << std::endl;
}

int main(int argc, char* argv[]) {
    std::string path;
    if (argc > 1) {
        path = argv[1];
    } else {
        std::cout << "Path with instance not specified" << std::endl;
        return 0;
    }

    if (path.back() != '/') {
        path += '/';
    }

    // SearchBestPso(path);
    // return 0;

    std::vector<TStat> statsACO;
    std::vector<TStat> statsPSO;

    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string filename = entry.path().filename();
            if (filename.find(".vrp") == std::string::npos) {
                continue;
            }
            filename = path + filename;

            // auto [reader, problem] = ReadInstance(filename);
            SI::CVRP::TFileInstanceReader reader(filename);
            auto [bestCntRoutes, bestDistance] = ReadBestSolution(filename);
            SI::CVRP::TProblem problem = reader.Read();

            std::cout << "Instance: " << reader.Name << std::endl;
            std::cout << reader.Comment << std::endl;

            SI::ACO::TParameters acoParameters;
            // acoParameters.AntsPerIteration = reader.Dimension * 2;
            acoParameters.AntsPerIteration = std::max(static_cast<int64_t>(100), reader.Dimension);

            SI::PSO::TParameters psoParameters;
            // psoParameters.SwarmSize = reader.Dimension * 2;
            psoParameters.SwarmSize = std::max(static_cast<int64_t>(100), reader.Dimension);

            std::thread threadACO([&](){
                auto start = std::chrono::high_resolution_clock::now();
                SI::CVRP::TSolution solutionACO = RunACO(problem, acoParameters);
                auto end = std::chrono::high_resolution_clock::now();

                statsACO.emplace_back(TStat{
                    reader.Name,
                    reader.Comment,
                    {bestCntRoutes, bestDistance},
                    {solutionACO.Routes().size(), solutionACO.TotalDistance() / SI::CVRP::TFileInstanceReader::MULTIPLIER},
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                });

                std::cout << "ACO solution: " << solutionACO.TotalDistance() / SI::CVRP::TFileInstanceReader::MULTIPLIER
                << " total distance and " << solutionACO.Routes().size() << " routes" << std::endl;
                std::cout << "Percent: " << statsACO.back().FindPercent() << std::endl;
            });

            std::thread threadPSO([&](){
                auto start = std::chrono::high_resolution_clock::now();
                SI::CVRP::TSolution solutionPSO = RunPSO(problem, psoParameters);
                auto end = std::chrono::high_resolution_clock::now();

                statsPSO.emplace_back(TStat{
                    reader.Name,
                    reader.Comment,
                    {bestCntRoutes, bestDistance},
                    {solutionPSO.Routes().size(), solutionPSO.TotalDistance() / SI::CVRP::TFileInstanceReader::MULTIPLIER},
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                });

                std::cout << "PSO solution: " << solutionPSO.TotalDistance() / SI::CVRP::TFileInstanceReader::MULTIPLIER
                << " total distance and " << solutionPSO.Routes().size() << " routes" << std::endl;
                std::cout << "Percent: " << statsPSO.back().FindPercent() << std::endl;
            });

            threadACO.join();
            threadPSO.join();

            std::cout << std::endl << "---------------------------------------------------------------------------------------------------------"
            << std::endl << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    int64_t worstACOIndex = -1;
    int64_t bestACOIndex = -1;
    int64_t worstPSOIndex = -1;
    int64_t bestPSOIndex = -1;
    double percentACO = 0;
    double percentPSO = 0;
    int64_t timeACO = 0;
    int64_t timePSO = 0;


    for (size_t i = 0; i < statsACO.size(); i++) {
        if (worstACOIndex == -1 || statsACO[i].FindPercent() > statsACO[worstACOIndex].FindPercent()) {
            worstACOIndex = i;
        }
        if (worstPSOIndex == -1 || statsPSO[i].FindPercent() > statsPSO[worstPSOIndex].FindPercent()) {
            worstPSOIndex = i;
        }
        if (bestACOIndex == -1 || statsACO[i].FindPercent() < statsACO[bestACOIndex].FindPercent()) {
            bestACOIndex = i;
        }
        if (bestPSOIndex == -1 || statsPSO[i].FindPercent() < statsPSO[bestPSOIndex].FindPercent()) {
            bestPSOIndex = i;
        }

        percentACO += statsACO[i].FindPercent();
        percentPSO += statsPSO[i].FindPercent();

        timeACO += statsACO[i].Time;
        timePSO += statsPSO[i].Time;
    }

    std::cout << "Worst ACO: " << statsACO[worstACOIndex].Name << std::endl << statsACO[worstACOIndex].Description << std::endl;
    std::cout << "Etalon: " << statsACO[worstACOIndex].Best.BestDistance << ". Found: " << statsACO[worstACOIndex].Mine.BestDistance << std::endl;
    std::cout << "Percent: " << statsACO[worstACOIndex].FindPercent() << std::endl;

    std::cout << std::endl;

    std::cout << "Best ACO: " << statsACO[bestACOIndex].Name << std::endl << statsACO[bestACOIndex].Description << std::endl;
    std::cout << "Etalon: " << statsACO[bestACOIndex].Best.BestDistance << ". Found: " << statsACO[bestACOIndex].Mine.BestDistance << std::endl;
    std::cout << "Percent: " << statsACO[bestACOIndex].FindPercent() << std::endl;
    std::cout << std::endl;

    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "Worst PSO: " << statsPSO[worstPSOIndex].Name << std::endl << statsPSO[worstPSOIndex].Description << std::endl;
    std::cout << "Best: " << statsPSO[worstPSOIndex].Best.BestDistance << ". Found: " << statsPSO[worstPSOIndex].Mine.BestDistance << std::endl;
    std::cout << "Percent: " << statsPSO[worstPSOIndex].FindPercent() << std::endl;

    std::cout << std::endl;

    std::cout << "Best PSO: " << statsPSO[bestPSOIndex].Name << std::endl << statsPSO[bestPSOIndex].Description << std::endl;
    std::cout << "Best: " << statsPSO[bestPSOIndex].Best.BestDistance << ". Found: " << statsPSO[bestPSOIndex].Mine.BestDistance << std::endl;
    std::cout << "Percent: " << statsPSO[bestPSOIndex].FindPercent() << std::endl;

    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "Average time ACO: " << timeACO / static_cast<double>(statsACO.size()) << std::endl;
    std::cout << "Average time PSO: " << timePSO / static_cast<double>(statsPSO.size()) << std::endl;

    std::cout << std::endl;

    std::cout << "Percent ACO: " << percentACO / static_cast<double>(statsACO.size()) << std::endl;
    std::cout << "Percent PSO: " << percentPSO / static_cast<double>(statsPSO.size()) << std::endl;



    return 0;
}