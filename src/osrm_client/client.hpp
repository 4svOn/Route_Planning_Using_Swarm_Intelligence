#pragma once

#include "request.hpp"
#include "response.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>

namespace osrm {

    class Client {
    public:

        static constexpr std::string_view kName = "osrm-client";

        Client(userver::clients::http::Client& httpClient);

        std::optional<ResponseRoute> MakeRouteRequest(const RequestRoute& request);

        std::optional<ResponseTable> MakeTableRequest(const RequestTable& request);

    private:
        template <typename TRequest, typename TResponse>
        std::optional<TResponse> MakeRequest(const TRequest& osrmRequest){
            userver::clients::http::Request request = HttpClient_.CreateRequest().get().timeout(1000).retry(2);
            request.url(osrmRequest.GetUrlFull());
            LOG_INFO() << "OSRM request: " << request.GetUrl();
            try {
                auto response = request.perform();
                // LOG_INFO() << (response->IsOk() ? "OSRM request was OK" : "OSRM request failed");
                // LOG_INFO() << "OSRM response: " << response->body();
                return TResponse{std::move(*response).body(), osrmRequest};  // no copying
            } catch (const userver::clients::http::TimeoutException&) {
                LOG_INFO() << "OSRM request timeout";
            }
            return {};
        }

    private:
        userver::clients::http::Client& HttpClient_;
    };

}
