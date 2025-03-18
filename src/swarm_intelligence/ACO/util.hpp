#pragma once

#include <cstdint>
#include <vector>

namespace SI::ACO {

    struct TParameters {
        double PheromonceImportance = 1.0; // Alpha
        double DistanceImportance = 2.0; // Beta
        double EvaporateRate = 0.1; // Rho
        double ProbabalisticBorder = 0.9; // Q
        int64_t AntsPerIteration = 25;
        int64_t MaxIterations = 1000;
    };

    using TPheromoneMatrix = std::vector<std::vector<double>>;
}