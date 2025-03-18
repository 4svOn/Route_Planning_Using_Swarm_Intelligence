#pragma once

#include <proto/cpp/common.pb.h>

#include <vector>
#include <string>

namespace base {
    struct Coordinate {
        Coordinate() = default;
        Coordinate(const pb::Coordinate& coordinate_pb);

        double Longitude;
        double Latitude;
    };

    using Coordinates = std::vector<Coordinate>;

    std::string ToString(const Coordinates& coordinates);
};
