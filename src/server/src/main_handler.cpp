#include "main_handler.hpp"

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/http/predefined_header.hpp>


#include <osrm_client/client.hpp>
#include <osrm_client/request.hpp>

#include <swarm_intelligence/ACO/ant.hpp>

#include "proto/cpp/request.pb.h"
#include "proto/cpp/response.pb.h"

#include <sstream>
#include <string>

namespace main_handler {

namespace {

    class MainHandler final : public userver::server::handlers::HttpHandlerBase {
    public:
        static constexpr std::string_view kName = "handler-hello";

        using HttpHandlerBase::HttpHandlerBase;

        MainHandler(const userver::components::ComponentConfig& config,
              const userver::components::ComponentContext& component_context)
            : HttpHandlerBase(config, component_context)
            , HttpClient_(component_context.FindComponent<userver::components::HttpClient>("http-client"))
        {}

        std::string HandleRequestThrow(
            const userver::server::http::HttpRequest& request,
            userver::server::request::RequestContext&) const override {
            auto& response = request.GetHttpResponse();
            response.SetHeader(userver::http::headers::PredefinedHeader{"Access-Control-Allow-Origin"}, "*");
            response.SetHeader(userver::http::headers::PredefinedHeader{"Access-Control-Allow-Methods"}, "GET, POST, PUT, DELETE, OPTIONS");
            response.SetHeader(userver::http::headers::kAccessControlAllowHeaders, "Content-Type, Cache-Control, Pragma, Expires");
            response.SetContentType("application/octet-stream");

            // LOG_INFO() << request.GetUrl();
            // LOG_INFO() << request.RequestBody();
            // LOG_INFO() << request_context.GetUserData<std::string>();

            if (request.GetMethod() == userver::server::http::HttpMethod::kPost) {
                osrm::Client osrmClient{HttpClient_.GetHttpClient()};
                pb::Request requestBodyPb;
                requestBodyPb.ParseFromString(request.RequestBody());
                LOG_INFO() << requestBodyPb.DebugString();

                auto coords = base::FromProto(requestBodyPb.markers());
                osrm::RequestRoute osrmRequestRoute{coords};
                auto osrmResponse = osrmClient.MakeRouteRequest(osrmRequestRoute);
                if (!osrmResponse) {
                    LOG_INFO() << "OSRM request failed";
                    throw userver::server::handlers::InternalServerError();
                }

                pb::Response response_pb;
                response_pb.set_polyline(osrmResponse->GetPolyline());
                // LOG_INFO() << "RESPONSE: " << response_pb.DebugString();
                std::string res;
                response_pb.SerializeToString(&res);
                return res;
            }
            return "";
        }

    private:
        userver::components::HttpClient& HttpClient_;
    };

} // namespace

std::string SayHelloTo(std::string_view name) {
    if (name.empty()) {
    name = "unknown user";
    }

    return fmt::format("MainHandler, {}!\n", name);
}

void AppendMainHandler(userver::components::ComponentList &component_list) {
    component_list.Append<MainHandler>();

    // std::string q = "ChIJ9mjCEIPBQkAROAWZTYneS0AKEgm6kK2eaMxCQBEs2eyP8NpLQBC2osGG0jIAChIJ9mjCEIPBQkAROAWZTYneS0AKEgm6kK2eaMxCQBEs2eyP8NpLQAoSCcJvjlSdyUJAEai7EpXU3EtAEIu8wYbSMg";
    // pb::Request requestBodyPb;
    // requestBodyPb.ParseFromString(q);
    // std::cout << "ABOBA: " << requestBodyPb.DebugString();
}

} // namespace main_handler
