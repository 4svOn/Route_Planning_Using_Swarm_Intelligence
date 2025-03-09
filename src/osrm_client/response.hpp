#pragma once

#include "util.hpp"
#include "request.hpp"

#include <userver/formats/json/value.hpp>

#include <string>

namespace osrm {

    class Response {
    public:
        // Response() = default;
        Response(std::string&& body);

        const std::string& GetCode() const;

    protected:
        userver::formats::json::Value JsonResponse_;

    private:
        std::string Code_;
    };

    class ResponseRoute : public Response {
    public:
        // ResponseRoute() = default;
        ResponseRoute(std::string&& body, const RequestRoute& request);

        const std::string& GetPolyline() const;
    private:
        const RequestRoute& Request_;

        std::string Polyline_;
    };

    class ResponseTable : public Response {
    public:
        // ResponseTable() = default;
        ResponseTable(std::string&& body, const RequestTable& request);

        const DurationTable& GetDurationTable() const;
        const DistanceTable& GetDistanceTable() const;
    private:
        const RequestTable& Request_;

        uint64_t Size_;
        DurationTable DurationTable_;
        DistanceTable DistanceTable_;
    };
}
