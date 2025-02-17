#pragma once

#include "osrm_util.hpp"

#include <base/coordinates.hpp>

#include <userver/clients/http/request.hpp>

#include <vector>
#include <string>

namespace osrm {

    class OSRMRequest {
    public:
        OSRMRequest() = default;

        virtual std::string GetUrlFull() const = 0;

    protected:
        std::string GetUrlBegin() const;

    private:
        OSRMServiceEnum Service_ = Route;
        OSRMProfileEnum Profile_ = Driving;
    private:
        base::Coordinates Coordinates_;
    };

    class OSRMRequestRoute : public OSRMRequest {
    public:
        OSRMRequestRoute() = default;

        std::string GetUrlFull() const override;

        // void FillUserverRequest(userver::clients::http::Request& request) const;

    private:
        std::string Overview_ = "full";
    };

}
