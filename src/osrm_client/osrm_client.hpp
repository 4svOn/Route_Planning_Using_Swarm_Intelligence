#pragma once

#include "osrm_request.hpp"
#include "osrm_response.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>

namespace osrm {

    class Client {
    public:

        static constexpr std::string_view kName = "osrm-client";

        Client(userver::clients::http::Client& httpClient);

        ResponseRoute MakeRouteRequest(const RequestRoute& request);

    private:
        userver::clients::http::Client& HttpClient_;
    };

}
