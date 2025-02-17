#include "osrm_response.hpp"

#include <userver/logging/log.hpp>

namespace osrm {
    OSRMResponse::OSRMResponse(std::string&& body) {
        auto jsonResponse = userver::formats::json::FromString(body);
        Code_ = jsonResponse["code"].As<std::string>();
        LOG_INFO() << "ABOBA: " << body;
    }

    std::string OSRMResponse::GetCode() const {
        return Code_;
    }
}
