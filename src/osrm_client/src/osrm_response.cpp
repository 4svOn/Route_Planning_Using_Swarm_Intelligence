#include "osrm_response.hpp"

#include <userver/logging/log.hpp>

namespace osrm {
    Response::Response(std::string&& body)
        : JsonResponse_(userver::formats::json::FromString(body))
    {
        Code_ = JsonResponse_["code"].As<std::string>();
        LOG_INFO() << "OSRM RESPONSE: " << body;
    }

    const std::string& Response::GetCode() const {
        return Code_;
    }

    ResponseRoute::ResponseRoute(std::string&& body) : Response(std::move(body)) {
        Polyline_ = JsonResponse_["routes"][0]["geometry"].As<std::string>();
    }

    const std::string& ResponseRoute::GetPolyline() const {
        return Polyline_;
    }
}
