#pragma once

#include <cstdint>
#include <vector>

namespace SI::PSO {

    using TCoordinate = double;

    struct TParameters {
        int64_t SwarmSize = 25;
        int64_t MaxIterations = 1000;
        TCoordinate W;
        TCoordinate C1;
        TCoordinate C2;
    };
}