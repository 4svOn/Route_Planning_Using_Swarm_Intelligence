#include "osrm_client.hpp"

namespace osrm {

    OSRMClient::OSRMClient(userver::clients::http::Client& httpClient)
        : HttpClient_(httpClient)
    {}

    OSRMResponse OSRMClient::GetRoute(const OSRMRequestRoute& osrmRequest) {
        userver::clients::http::Request request = HttpClient_.CreateRequest().get().timeout(1000).retry(2);
        request.url(osrmRequest.GetUrlFull());
        LOG_INFO() << "OSRM request: " << request.GetUrl();
        try {
            auto response = request.perform();
            if (response->IsOk()) {
                LOG_INFO() << "OSRM response: " << response->body();
                return OSRMResponse{std::move(*response).body()};  // no copying
            }
            LOG_INFO() << "OSRM request failed";
            LOG_INFO() << response->status_code();
            LOG_INFO() << response->body();
        } catch (const userver::clients::http::TimeoutException&) {
            LOG_INFO() << "OSRM request timeout";
        }
        return {};
    }
}
