#include "main_handler.hpp"

#include <fmt/format.h>

#include <userver/components/component_context.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/http/predefined_header.hpp>
#include <userver/utils/async.hpp>

#include <osrm_client/client.hpp>
#include <osrm_client/request.hpp>

#include <swarm_intelligence/ACO/solver.hpp>
#include <swarm_intelligence/ACO/util.hpp>

#include <swarm_intelligence/PSO/solver.hpp>
#include <swarm_intelligence/PSO/util.hpp>

#include <swarm_intelligence/common/CVRP_problem.hpp>
#include <swarm_intelligence/common/node.hpp>

#include "proto/cpp/common.pb.h"
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
        static constexpr std::string_view kName = "handler-cvrp-solve";

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

            if (request.GetMethod() == userver::server::http::HttpMethod::kPost) {
                try {
                    pb::Request requestBodyPb = ParseRequestBody(request.RequestBody());
                    SI::CVRP::TNodesWithCoordinates nodes{requestBodyPb};
                    osrm::ResponseTable tableResponse = SendTableRequest(nodes);
                    SI::CVRP::TProblem problem{nodes, tableResponse.GetDurationTable(), tableResponse.GetDistanceTable()};
                    pb::Response response_pb = PrepareResponse(SolveCVRP(problem, requestBodyPb.algorithm()));

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

        pb::Response PrepareResponse(const std::vector<std::pair<pb::Algorithm, SI::CVRP::TSolution>>& algoSolutions) const {
            pb::Response response_pb;
            SI::TDistance bestTotalDistance = -1;
            for (const auto& algoSolution : algoSolutions) {
                pb::CVRP_Solution* solution_pb = nullptr;

                if (algoSolution.first == pb::Algorithm::ACO) {
                    solution_pb = response_pb.mutable_aco_solution();
                } else if (algoSolution.first == pb::Algorithm::PSO) {
                    solution_pb = response_pb.mutable_pso_solution();

                }
                assert(solution_pb != nullptr);

                const auto& solution = algoSolution.second;

                if (solution.TotalDistance() < bestTotalDistance || bestTotalDistance == -1) {
                    bestTotalDistance = solution.TotalDistance();
                    response_pb.set_better_algorithm(algoSolution.first);
                }
                solution_pb->set_algorithm(algoSolution.first);

                solution_pb->set_total_distance(solution.TotalDistance());

                std::vector<userver::engine::TaskWithResult<std::pair<std::vector<int64_t>, osrm::ResponseRoute>>> tasks;
                tasks.reserve(solution.Routes().size());
                LOG_INFO() << "Algorithm: " << algoSolution.first;
                LOG_INFO() << "Nodes count: " << algoSolution.second.Problem().Nodes().Nodes().size();
                LOG_INFO() << "Coordinates count: " << algoSolution.second.Problem().Nodes().Coordinates().size();

                for (const auto& route : solution.Routes()) {
                    tasks.push_back(userver::utils::Async("OSRM_route_request", [route = std::move(route), &solution, this] {
                        base::Coordinates routeCoordinates;
                        std::vector<int64_t> routeUIDs;
                        for (const auto& node : solution.RouteWithCoordinates(route)) {
                            routeCoordinates.emplace_back(node.Coordinate);
                            routeUIDs.push_back(node.UIDFromFrontend);
                        }
                        return std::make_pair(routeUIDs, SendRouteRequest(routeCoordinates));
                    }));
                }


                for (auto& task : tasks) {
                    auto [routeUIDs, routeResponse] = task.Get();
                    auto* route_pb = solution_pb->add_routes();
                    route_pb->set_polyline(routeResponse.GetPolyline());
                    for (const auto& uid : routeUIDs) {
                        route_pb->add_uids(uid);
                    }
                }
                LOG_INFO() << "ACO algo: " << response_pb.aco_solution().algorithm();
                LOG_INFO() << "PSO algo: " << response_pb.pso_solution().algorithm();
            }

            LOG_INFO() << "Response: " << response_pb.DebugString();
            LOG_INFO() << "Better solution: " << response_pb.better_algorithm();

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

        std::vector<std::pair<pb::Algorithm, SI::CVRP::TSolution>> SolveCVRP(const SI::CVRP::TProblem& problem, pb::Algorithm algo) const {
            std::vector<userver::engine::TaskWithResult<std::pair<pb::Algorithm, SI::CVRP::TSolution>>> tasks;

            if (algo == pb::Algorithm::ACO || algo == pb::Algorithm::BOTH) {
                tasks.push_back(userver::utils::Async("CVRP_SOLVING", [&problem, this] {
                    return std::make_pair(pb::Algorithm::ACO, SI::ACO::TSolver{problem, SI::ACO::TParameters{}}.Solve());
                }));
            }
            
            if (algo == pb::Algorithm::PSO || algo == pb::Algorithm::BOTH) {
                tasks.push_back(userver::utils::Async("CVRP_SOLVING", [&problem, this] {
                    return std::make_pair(pb::Algorithm::PSO, SI::PSO::TSolver{problem, SI::PSO::TParameters{}}.Solve());
                }));
            }

            std::vector<std::pair<pb::Algorithm, SI::CVRP::TSolution>> solutions;
            for (auto& task : tasks) {
                solutions.push_back(task.Get());
            }
            return solutions;
        }

        // SI::CVRP::TProblem SolveByACO(const SI::CVRPLibrary::TProblem& problem)

        // SI::CVRP::TProblem SendCVRPRequest(const osrm::ResponseTable& responseTable) const {

        // }

    private:
        userver::components::HttpClient& HttpClient_;
    };

} // namespace

void AppendMainHandler(userver::components::ComponentList &component_list) {
    component_list.Append<MainHandler>();

    // std::string q = "ChIJ9mjCEIPBQkAROAWZTYneS0AKEgm6kK2eaMxCQBEs2eyP8NpLQBC2osGG0jIAChIJ9mjCEIPBQkAROAWZTYneS0AKEgm6kK2eaMxCQBEs2eyP8NpLQAoSCcJvjlSdyUJAEai7EpXU3EtAEIu8wYbSMg";
    // pb::Request requestBodyPb;
    // requestBodyPb.ParseFromString(q);
    // std::cout << "ABOBA: " << requestBodyPb.DebugString();
}

} // namespace main_handler
