#pragma once

#include <userver/formats/json/value.hpp>

#include <string>

namespace osrm {

    class Response {
    public:
        Response() = default;
        Response(std::string&& body);

        const std::string& GetCode() const;

    protected:
        userver::formats::json::Value JsonResponse_;

    private:
        std::string Code_;
    };

    class ResponseRoute : public Response {
    public:
        ResponseRoute() = default;
        ResponseRoute(std::string&& body);

        const std::string& GetPolyline() const;
    private:
        std::string Polyline_;
    };
}
