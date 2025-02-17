#pragma once

#include <string>
#include <map>

namespace osrm {
    const std::string OSRM_API_DOMAIN = "http://router.project-osrm.org";
    const std::string OSRM_API_VERSION = "v1";

    enum OSRMServiceEnum {
        Route = 1,
        Table = 2
    };

    enum OSRMProfileEnum {
        Driving = 1,
        Walking = 2,
        Bicycling = 3
    };

    std::string GetServiceName(OSRMServiceEnum service);
    std::string GetProfileName(OSRMProfileEnum profile);
};