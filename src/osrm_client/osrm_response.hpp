#pragma once

#include <userver/formats/json/value.hpp>

#include <string>

namespace osrm {

    class OSRMResponse {
    public:
        OSRMResponse() = default;
        OSRMResponse(std::string&& body);

        std::string GetCode() const;

    private:
        std::string Code_;
    };

}
