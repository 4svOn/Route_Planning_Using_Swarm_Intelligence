#pragma once

#include "ant.hpp"
#include "util.hpp"

#include <swarm_intelligence/common/CVRP_problem.hpp>

#include <vector>

namespace SI::ACO {
    class TSolver {
    public:
        TSolver(const CVRP::TProblem& p, const TParameters& parameters);
        CVRP::TSolution Solve();
        TDistance GetBestDistance() const;

    private:
        void InitializePheromones(); // Set τ₀ based on a heuristic
        void EvaporatePheromones(const TAnt& ant); // local update
        void DepositPheromones(const TAnt& ant); // global update

    private:
        CVRP::TProblem Problem_;
        TParameters Parameters_;
        std::vector<TAnt> Ants_;
        TPheromoneMatrix Pheromones_; // Pheromone matrix
        double BasePheromone_{1.0};
        TDistance BestDistance_{-1};
        int64_t BestAntIndex_{-1};
    };
}