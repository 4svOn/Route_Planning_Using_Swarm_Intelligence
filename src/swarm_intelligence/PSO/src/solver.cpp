#include "solver.hpp"

// #include <userver/logging/log.hpp>

#include <base/random.hpp>

#include <swarm_intelligence/common/two_opt.hpp>

#include <cassert>

namespace SI::PSO {
    TSolver::TSolver(const CVRP::TProblem& problem, const TParameters& parameters)
        : Problem_(problem)
        , Parameters_(parameters)
        , W_STEP_((TParameters::MAX_W - TParameters::MIN_W) / static_cast<double>(Parameters_.MaxIterations))
    {
        Particles_.reserve(Parameters_.SwarmSize);
        for (int i = 0; i < Parameters_.SwarmSize; ++i) {
            Particles_.emplace_back(Problem_);
        }
    }

    CVRP::TSolution TSolver::Solve() {
        int64_t countSameResult = 0;
        for (int i = 0; i < Parameters_.MaxIterations; ++i) {
            for (auto& particle : Particles_) {
                particle.Evaluate();
                if (RandomUniform() < 0.1) {
                    particle.MakeTwoOpt();
                }
                if (particle.BestDistance() < BestDistance_ || BestPosition_.empty()) {
                    BestDistance_ = particle.BestDistance();
                    BestPosition_ = particle.BestPosition();
                    BestRoutes_ = particle.BestRoutes();
                    countSameResult = 0;
                } else {
                    countSameResult++;
                }
            }
            BestDistance_ = TParticle::MakeTwoOpt(Problem_, BestRoutes_, BestDistance_);

            Parameters_.W -= W_STEP_;
            Parameters_.C1 = (1. - Parameters_.W) * RandomUniform();
            Parameters_.C2 = 1. - Parameters_.W - Parameters_.C1;
            for (auto& particle : Particles_) {
                particle.Update(Parameters_, BestPosition_);
            }

            if (countSameResult > Parameters_.MaxIterationsWitoutImprovement * Parameters_.SwarmSize) {
                break;
            }
        }

        return {Problem_, BestRoutes_, BestDistance_};
    }
}