#include "client.hpp"

namespace osrm {

    Client::Client(userver::clients::http::Client& httpClient)
        : HttpClient_(httpClient)
    {}

    std::optional<ResponseRoute> Client::MakeRouteRequest(const RequestRoute& request) {
        return MakeRequest<RequestRoute, ResponseRoute>(request);
    }

    std::optional<ResponseTable> Client::MakeTableRequest(const RequestTable& request) {
        return MakeRequest<RequestTable, ResponseTable>(request);
    }
}
