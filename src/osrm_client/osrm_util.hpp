#pragma once

#include <string>
#include <map>

namespace osrm {
    const std::string API_DOMAIN = "http://router.project-osrm.org";
    const std::string API_VERSION = "v1";

    enum ServiceEnum {
        Route = 1,
        Table = 2
    };

    enum ProfileEnum {
        Driving = 1,
        Walking = 2,
        Bicycling = 3
    };

    std::string GetServiceName(ServiceEnum service);
    std::string GetProfileName(ProfileEnum profile);
};