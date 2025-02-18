#include "osrm_util.hpp"

namespace osrm {
    std::string GetServiceName(ServiceEnum service) {
        switch (service) {
            case ServiceEnum::Route:
                return "route";
            case ServiceEnum::Table:
                return "table";
            default:
                return "";
        }
    }

    std::string GetProfileName(ProfileEnum profile) {
        switch (profile) {
            case ProfileEnum::Driving:
                return "driving";
            case ProfileEnum::Walking:
                return "walking";
            case ProfileEnum::Bicycling:
                return "bicycling";
            default:
                return "";
        }
    }
};