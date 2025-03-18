#pragma once

#include "util.hpp"
#include "request.hpp"

#include <swarm_intelligence/common/types.hpp>

#include <userver/formats/json/value.hpp>

#include <string>

using namespace SI;

namespace osrm {

    class Response {
    public:
        // Response() = default;
        Response(std::string&& body);

        std::string GetLogString() const;

        bool IsOk() const;

    protected:
        userver::formats::json::Value JsonResponse_;

    private:
        std::string Code_;
        std::string Message_;
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

        const RequestTable& GetRequest() const;

        const uint64_t& GetSize() const;
        const TDurationTable& GetDurationTable() const;
        const TDistanceTable& GetDistanceTable() const;
    private:
        const RequestTable& Request_;

        uint64_t Size_;
        TDurationTable DurationTable_;
        TDistanceTable DistanceTable_;
    };
}
