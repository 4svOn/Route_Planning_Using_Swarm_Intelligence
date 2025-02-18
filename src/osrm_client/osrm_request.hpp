#pragma once

#include "osrm_util.hpp"

#include <base/coordinates.hpp>

#include <userver/clients/http/request.hpp>

#include <vector>
#include <string>

namespace osrm {

    class Request {
    public:
        Request() = default;
        Request(const base::Coordinates& coordinates);

        virtual std::string GetUrlFull() const = 0;

    protected:
        std::string GetUrlBegin() const;

    private:
        ServiceEnum Service_ = Route;
        ProfileEnum Profile_ = Driving;
    private:
        base::Coordinates Coordinates_;
    };

    class RequestRoute : public Request {
    public:
        RequestRoute() = default;
        RequestRoute(const base::Coordinates& coordinates);

        std::string GetUrlFull() const override;

    private:
        std::string Overview_ = "full";
    };

}
