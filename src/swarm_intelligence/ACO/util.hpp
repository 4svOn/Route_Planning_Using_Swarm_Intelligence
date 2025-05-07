#pragma once

#include <cstdint>
#include <vector>

namespace SI::ACO {

    struct TParameters {
        double PheromonceImportance = 1.; // Alpha
        double DistanceImportance = 5.; // Beta
        double EvaporateRate = 0.01; // Rho
        double ProbabalisticBorder = 0.5; // Q
        int64_t AntsPerIteration = 50;
        int64_t MaxIterations = 5000;
        int64_t MaxIterationsWithoutImprovement = 500;
        // double G = 2.0;
        // double F = 2.0;
    };

    using TPheromoneMatrix = std::vector<std::vector<double>>;
}