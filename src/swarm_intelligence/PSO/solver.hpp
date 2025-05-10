#pragma once

#include "particle.hpp"
#include "util.hpp"

#include <swarm_intelligence/common/CVRP_problem.hpp>

#include <vector>

namespace SI::PSO {
    class TSolver {
    public:
        TSolver(const CVRP::TProblem& p, const TParameters& parameters);
        CVRP::TSolution Solve();
        TDistance GetBestDistance() const;

    private:
        CVRP::TRoutes ConstructBestRoutes() const;

    private:
        const CVRP::TProblem& Problem_;
        TParameters Parameters_;

        std::vector<TParticle> Particles_;
        TCoordinates BestPosition_;
        TDistance BestDistance_;
    };
}