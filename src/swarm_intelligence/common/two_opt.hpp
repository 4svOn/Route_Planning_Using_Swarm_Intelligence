#pragma once

#include "CVRP_problem.hpp"
#include "types.hpp"
#include "node.hpp"

namespace SI {
    std::vector<std::string> TwoOpt(const CVRP::TProblem& problem, CVRP::TRoutes& routes);
}