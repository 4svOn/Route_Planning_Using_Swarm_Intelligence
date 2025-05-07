#include "two_opt.hpp"

#include <vector>
#include <sstream>

namespace SI {
    std::vector<std::string> TwoOpt(const CVRP::TProblem& problem, CVRP::TRoutes& routes) {
        bool improved = true;

        std::vector<std::string> logs;

        while (improved) {
            improved = false;
            int INDEX = 0;
            for (auto& route : routes) {
                for (auto i = 1; i < route.size() - 2; ++i) {
                    for (auto j = i + 2; j < route.size() - 2; ++j) {
                        auto currentDistance = problem.Distance(route[i], route[i + 1]) + problem.Distance(route[j], route[j + 1]);
                        auto newDistance = problem.Distance(route[i], route[j]) + problem.Distance(route[i + 1], route[j + 1]);

                        if (newDistance < currentDistance) {
                            logs.emplace_back("route: " + std::to_string(INDEX));
                            logs.emplace_back("newDistance < currentDistance: " + std::to_string(newDistance) + " < " + std::to_string(currentDistance));
                            logs.emplace_back("i: " + std::to_string(i) + " j: " + std::to_string(j));
                            auto makeLog = [&]() {
                                std::stringstream ss;
                                for (auto k = i; k <= j + 1; ++k) {
                                    ss << route[k].ID << " ";
                                }
                                logs.emplace_back(ss.str());
                            };

                            makeLog();

                            std::reverse(route.begin() + i + 1, route.begin() + j + 1);

                            makeLog();

                            logs.emplace_back("\n");

                            improved = true;
                            break;
                        }
                    }
                    if (improved) {
                        break;
                    }
                }
                if (improved) {
                    break;
                }
            }
            ++INDEX;
        }
        return logs;
    }
}

