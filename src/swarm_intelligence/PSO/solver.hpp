#pragma once

#include "particle.hpp"
#include "util.hpp"

#include <swarm_intelligence/common/CVRP_problem.hpp>
#include <swarm_intelligence/common/node.hpp>

#include <vector>

namespace SI::PSO {
    class TSolver {
    public:
        TSolver(const CVRP::TProblem& p, const TParameters& parameters);
        CVRP::TSolution Solve();
        TDistance GetBestDistance() const;

    private:
        const CVRP::TProblem& Problem_;
        TParameters Parameters_;

        std::vector<TParticle> Particles_;

        CVRP::TRoutes BestRoutes_;
        TCoordinates BestPosition_;
        TDistance BestDistance_;

        double W_STEP_;
    };
}