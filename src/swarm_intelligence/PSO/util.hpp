#pragma once

#include <cstdint>
#include <vector>

namespace SI::PSO {

    using TCoordinate = int32_t;

    struct TParameters {
        static constexpr double MAX_W = 0.6;
        static constexpr double MIN_W = 0.1;

        int64_t SwarmSize = 25;
        int64_t MaxIterations = 10000;
        double W = 0.1;
        double C1 = 0.45;
        double C2 = 0.45;
        double M = 0.1;
        int64_t MaxIterationsWitoutImprovement = 1000;
    };

    // double W = 0.15;
    // double C1 = 0.45;
    // double C2 = 0.4;
}