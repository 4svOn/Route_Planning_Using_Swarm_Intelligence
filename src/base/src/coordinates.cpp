
#include "coordinates.hpp"

#include <iomanip>
#include <sstream>

namespace base {

    Coordinate::Coordinate(const pb::Coordinate& coordinate_pb)
        : Longitude(coordinate_pb.longitude())
        , Latitude(coordinate_pb.latitude())
    {
    }

    std::string ToString(const Coordinates& coordinates) {
        std::stringstream ss;
        for (const auto &coordinate : coordinates) {
            ss << std::fixed << std::setprecision(8) << coordinate.Longitude << "," << coordinate.Latitude << ";";
        }
        auto res = ss.str();
        if (!res.empty()) {
            res.pop_back();
        }
        return res;
    }
}