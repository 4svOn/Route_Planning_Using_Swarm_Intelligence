#include "osrm_util.hpp"

namespace osrm {
    std::string GetServiceName(OSRMServiceEnum service) {
        switch (service) {
            case OSRMServiceEnum::Route:
                return "route";
            case OSRMServiceEnum::Table:
                return "table";
            default:
                return "";
        }
    }

    std::string GetProfileName(OSRMProfileEnum profile) {
        switch (profile) {
            case OSRMProfileEnum::Driving:
                return "driving";
            case OSRMProfileEnum::Walking:
                return "walking";
            case OSRMProfileEnum::Bicycling:
                return "bicycling";
            default:
                return "";
        }
    }
};