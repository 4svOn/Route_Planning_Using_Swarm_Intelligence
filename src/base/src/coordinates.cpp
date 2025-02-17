#include "coordinates.hpp"

#include <iomanip>
#include <sstream>

namespace base {
    std::string ToString(const Coordinates& coordinates) {
        std::stringstream ss;
        for (const auto &coordinate : coordinates) {
            ss << std::fixed << std::setprecision(8) << coordinate.Longitude << "," << coordinate.Latitude;
        }
        return ss.str();
    }
}