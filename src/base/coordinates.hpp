#pragma once

#include <vector>
#include <string>

namespace base {
    struct Coordinate {
        double Longitude;
        double Latitude;
    };

    using Coordinates = std::vector<Coordinate>;

    std::string ToString(const Coordinates& coordinates);
};
