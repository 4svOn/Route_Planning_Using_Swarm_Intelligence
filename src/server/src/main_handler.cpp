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

#include <swarm_intelligence/ACO/solver.hpp>
#include <swarm_intelligence/ACO/util.hpp>
#include <swarm_intelligence/common/CVRP_problem.hpp>
#include <swarm_intelligence/common/node.hpp>

#include "proto/cpp/request.pb.h"
#include "proto/cpp/response.pb.h"

#include <sstream>
#include <string>

namespace main_handler {

namespace {
    void DEBUG_INFO(const std::string& message) {
        std::cout << message << std::endl;
        std::cout.flush();
    }

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
                pb::Request requestBodyPb = ParseRequestBody(request.RequestBody());

                SI::CVRP::TNodesWithCoordinates nodes = {requestBodyPb};

                osrm::ResponseTable tableResponse = SendTableRequest(nodes);
                SI::CVRP::TProblem problem{nodes, tableResponse.GetDurationTable(), tableResponse.GetDistanceTable()};
                try {
                    SI::CVRP::TSolution solution = SolveByACO(problem);
                    pb::Response response_pb = PrepareResponse(solution);
                    // LOG_INFO() << "RESPONSE: " << response_pb.DebugString();
                    std::string res;
                    response_pb.SerializeToString(&res);
                    return res;
                } catch (const std::exception& e) {
                    throw;
                }


            }
            return "";
        }

        osrm::ResponseRoute SendRouteRequest(const base::Coordinates& routeCoordinates) const {
            osrm::Client osrmClient{HttpClient_.GetHttpClient()};
            osrm::RequestRoute osrmRequestRoute{routeCoordinates};
            auto osrmResponse = osrmClient.MakeRouteRequest(osrmRequestRoute);
            if (!osrmResponse || !osrmResponse->IsOk()) {
                LOG_INFO() << "OSRM request for route failed";
                throw userver::server::handlers::InternalServerError();
            }
            return *osrmResponse;
        }

        pb::Request ParseRequestBody(const std::string& requestBody) const {
            pb::Request requestBodyPb;
            requestBodyPb.ParseFromString(requestBody);
            return requestBodyPb;
        }

        pb::Response PrepareResponse(const SI::CVRP::TSolution& solution) const {
            pb::Response response_pb;
            response_pb.set_total_distance(solution.TotalDistance());
            for (const auto& route : solution.Routes()) {
                base::Coordinates routeCoordinates;
                for (const auto& node : solution.RouteWithCoordinates(route)) {
                    routeCoordinates.emplace_back(node.Coordinate);
                }
                osrm::ResponseRoute osrmResponseRoute = SendRouteRequest(routeCoordinates);
                response_pb.add_routes()->set_polyline(osrmResponseRoute.GetPolyline());
            }
            return response_pb;
        }

        osrm::ResponseTable SendTableRequest(const SI::CVRP::TNodesWithCoordinates& nodes) const {
            osrm::Client osrmClient{HttpClient_.GetHttpClient()};
            osrm::RequestTable osrmRequestTable{nodes};
            auto osrmResponse = osrmClient.MakeTableRequest(osrmRequestTable);
            if (!osrmResponse || !osrmResponse->IsOk()) {
                LOG_INFO() << "OSRM request for table failed";
                throw userver::server::handlers::InternalServerError();
            }
            return *osrmResponse;
        }

        SI::CVRP::TSolution SolveByACO(const SI::CVRP::TProblem& problem) const {
            return SI::ACO::TSolver{problem, SI::ACO::TParameters{}}.Solve();
        }



        // SI::CVRP::TProblem SolveByACO(const SI::CVRPLibrary::TProblem& problem)

        // SI::CVRP::TProblem SendCVRPRequest(const osrm::ResponseTable& responseTable) const {

        // }

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
