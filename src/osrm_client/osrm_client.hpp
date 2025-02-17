#pragma once

#include "osrm_request.hpp"
#include "osrm_response.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>

namespace osrm {

    class OSRMClient {
    public:

        static constexpr std::string_view kName = "osrm-client";

        OSRMClient(userver::clients::http::Client& httpClient);

        OSRMResponse GetRoute(const OSRMRequestRoute& request);

    private:
        userver::clients::http::Client& HttpClient_;
    };

}
