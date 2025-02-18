#include "osrm_client.hpp"

namespace osrm {

    Client::Client(userver::clients::http::Client& httpClient)
        : HttpClient_(httpClient)
    {}

    ResponseRoute Client::MakeRouteRequest(const RequestRoute& osrmRequest) {
        userver::clients::http::Request request = HttpClient_.CreateRequest().get().timeout(1000).retry(2);
        request.url(osrmRequest.GetUrlFull());
        LOG_INFO() << "OSRM request: " << request.GetUrl();
        try {
            auto response = request.perform();
            LOG_INFO() << (response->IsOk() ? "OSRM request was OK" : "OSRM request failed");
            LOG_INFO() << "OSRM response: " << response->body();
            return ResponseRoute{std::move(*response).body()};  // no copying
        } catch (const userver::clients::http::TimeoutException&) {
            LOG_INFO() << "OSRM request timeout";
        }
        return {};
    }
}
