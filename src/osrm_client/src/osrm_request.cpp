#include "osrm_request.hpp"

#include <userver/http/url.hpp>

namespace osrm {

    std::string OSRMRequest::GetUrlBegin() const {
        std::string url = OSRM_API_DOMAIN;
        url += "/" + GetServiceName(Service_);
        url += "/" + OSRM_API_VERSION;
        url += "/" + GetProfileName(Profile_);
        url += "/" + base::ToString(Coordinates_);
        return url;
    }

    // OSRMRequestRoute ----------------------------------------------------------------
    std::string OSRMRequestRoute::GetUrlFull() const {
        userver::http::Args args;
        args["overview"] = Overview_;

        auto url = userver::http::MakeUrl(OSRMRequest::GetUrlBegin(), args);
        // return userver::http::UrlEncode(url);
        return url;
    }

    // void OSRMRequestRoute::FillUserverRequest(userver::clients::http::Request& request) const {

    // }
    // ---------------------------------------------------------------------------------

}