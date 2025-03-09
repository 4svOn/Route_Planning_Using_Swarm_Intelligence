#include "request.hpp"

#include <userver/http/url.hpp>

namespace osrm {

    // Request ---------------------------------------------------------------------
    Request::Request(const base::Coordinates& coordinates, ServiceEnum service, ProfileEnum profile)
        : Coordinates_(coordinates)
        , Service_(service)
        , Profile_(profile)
    {
    }

    std::string Request::GetUrlBegin() const {
        std::string url = API_DOMAIN;
        url += "/" + GetServiceName(Service_);
        url += "/" + API_VERSION;
        url += "/" + GetProfileName(Profile_);
        url += "/" + base::ToString(Coordinates_);
        return url;
    }
    // -----------------------------------------------------------------------------

    // RequestRoute ----------------------------------------------------------------
    RequestRoute::RequestRoute(const base::Coordinates& coordinates)
        : Request(coordinates, Route, Driving)
    {
    }

    std::string RequestRoute::GetUrlFull() const {
        userver::http::Args args;
        args["overview"] = Overview_;

        auto url = userver::http::MakeUrl(Request::GetUrlBegin(), args);
        // return userver::http::UrlEncode(url);
        return url;
    }
    // -----------------------------------------------------------------------------

    // RequestTable ----------------------------------------------------------------
    RequestTable::RequestTable(const base::Coordinates& coordinates)
        : Request(coordinates, Table, Driving)
    {
    }

    std::string RequestTable::GetUrlFull() const {
        userver::http::Args args;
        args["annotation"] = Annotation_;

        auto url = userver::http::MakeUrl(Request::GetUrlBegin(), args);
        // return userver::http::UrlEncode(url);
        return url;
    }
    // -------------------------------------------------------------------------------

}