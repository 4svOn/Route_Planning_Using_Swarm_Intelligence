#pragma once

#include "util.hpp"

#include <swarm_intelligence/common/node.hpp>

#include <userver/clients/http/request.hpp>

#include <cstdint>
#include <vector>
#include <string>

namespace osrm {

    class Request {
    public:
        Request() = default;
        Request(const base::Coordinates& coordinates, ServiceEnum service, ProfileEnum profile);

        virtual std::string GetUrlFull() const = 0;

        uint64_t GetSize() const;

    protected:
        std::string GetUrlBegin() const;

    private:
        base::Coordinates Coordinates_;
        ServiceEnum Service_;
        ProfileEnum Profile_;
    };

    class RequestRoute : public Request {
    public:
        RequestRoute() = default;
        RequestRoute(const base::Coordinates& coordinates);

        std::string GetUrlFull() const override;

    private:
        std::string Overview_ = "full";
    };

    class RequestTable : public Request {
        public:
            RequestTable() = default;
            RequestTable(const SI::CVRP::TNodesWithCoordinates& nodes);

            std::string GetUrlFull() const override;

        private:
            std::string Annotations_ = "duration,distance";
        };

}
