#pragma once

#include "util.hpp"

#include <base/ranges.hpp>

#include <swarm_intelligence/common/CVRP_problem.hpp>

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_set>

namespace SI::ACO {
    class TAnt {
    public:
        TAnt(const CVRP::TProblem& problem);
        void Reset();  // Reset for next iteration

        TDistance TotalDistance() const;
        const CVRP::TRoutes& GetRoutes() const;

        void ConstructSolution(const TParameters& parameters, const TPheromoneMatrix& pheromones); // Build routes probabilistically

    private:
        // std::vector<size_t> GetAllFeasibleUnvisitedCustomers() const;
        void VisitCustomer(const CVRP::TNode& customer);
        void ReturnToDepot();
        CVRP::TNode SelectNextCustomer(const TSubrange<CVRP::TNodesSet::iterator>& feasibleCustomers, const TParameters& parameters, const TPheromoneMatrix& pheromones) const;

    private:
        const CVRP::TProblem& Problem_;
        CVRP::TRoutes Routes_; // Routes for multiple vehicles
        CVRP::TNodesSet VisitedCustomers_; // Track visited customers
        CVRP::TNodesSet UnvisitedCustomers_; // Track unvisited customers
        int64_t RemainingCapacity_; // Current vehicle's remaining capacity
    };
};