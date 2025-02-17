#include "hello.hpp"

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/http/predefined_header.hpp>

#include <osrm_client/osrm_client.hpp>
#include <osrm_client/osrm_request.hpp>

#include <swarm_intelligence/ACO/ant.hpp>

#include "proto/cpp/request.pb.h"

namespace service_template {

namespace {

    class Hello final : public userver::server::handlers::HttpHandlerBase {
    public:
        static constexpr std::string_view kName = "handler-hello";

        using HttpHandlerBase::HttpHandlerBase;

        Hello(const userver::components::ComponentConfig &config,
              const userver::components::ComponentContext &component_context)
            : HttpHandlerBase(config, component_context)
            , HttpClient_(component_context.FindComponent<userver::components::HttpClient>("http-client"))
        {}

        std::string HandleRequestThrow(
            const userver::server::http::HttpRequest &request,
            userver::server::request::RequestContext &) const override {
            auto& response = request.GetHttpResponse();
            response.SetContentType("text/plain");
            response.SetHeader(userver::http::headers::PredefinedHeader{"Access-Control-Allow-Origin"}, "*");
            response.SetHeader(userver::http::headers::PredefinedHeader{"Access-Control-Allow-Methods"}, "GET, POST, PUT, DELETE, OPTIONS");
            response.SetHeader(userver::http::headers::kAccessControlAllowHeaders, "Content-Type");

            if (request.GetMethod() == userver::server::http::HttpMethod::kPost) {
                osrm::OSRMClient osrmClient{HttpClient_.GetHttpClient()};
                request_pb::Request requestBodyPb;
                requestBodyPb.ParseFromString(request.RequestBody());
                LOG_INFO() << requestBodyPb.DebugString();

                osrm::OSRMRequestRoute osrmRequestRoute;
                auto osrmResponse = osrmClient.GetRoute(osrmRequestRoute);
                LOG_INFO() << "OSRM RESPONSE: " << osrmResponse.GetCode();
                return requestBodyPb.DebugString();
            }
            if (request.GetMethod() == userver::server::http::HttpMethod::kGet) {
                return service_template::SayHelloTo(request.GetArg("name"));
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

    return fmt::format("Hello, {}!\n", name);
}

void AppendHello(userver::components::ComponentList &component_list) {
    component_list.Append<Hello>();
}

} // namespace service_template
