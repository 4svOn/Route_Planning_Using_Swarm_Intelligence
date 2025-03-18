#pragma once

#include <cstdint>
#include <vector>

namespace SI {
    using TNodeID = uint64_t;
    using TDistance = int64_t;
    using TDuration = int64_t;
    using TCapacity = int64_t;

    using TCapacities = std::vector<TCapacity>;
    using TDurationTable = std::vector<std::vector<TDuration>>;
    using TDistanceTable = std::vector<std::vector<TDistance>>;
};