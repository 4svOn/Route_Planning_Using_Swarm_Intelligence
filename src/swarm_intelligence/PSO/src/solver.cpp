#include "solver.hpp"

// #include <userver/logging/log.hpp>

#include <base/random.hpp>

#include <swarm_intelligence/common/two_opt.hpp>

#include <cassert>

namespace SI::PSO {
    TSolver::TSolver(const CVRP::TProblem& problem, const TParameters& parameters)
        : Problem_(problem)
        , Parameters_(parameters)
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
                    countSameResult = 0;
                } else {
                    countSameResult++;
                }
            }
            BestDistance_ = TParticle::MakeTwoOpt(Problem_, BestPosition_, BestDistance_);

            for (auto& particle : Particles_) {
                particle.Update(Parameters_, BestPosition_);
            }

            if (countSameResult > Parameters_.MaxIterationsWitoutImprovement * Parameters_.SwarmSize) {
                break;
            }
        }

        return {Problem_, ConstructBestRoutes(), BestDistance_};
    }

    TDistance TSolver::GetBestDistance() const {
        return BestDistance_;
    }

    CVRP::TRoutes TSolver::ConstructBestRoutes() const {
        TParticle particle{Problem_, BestPosition_};
        TDistance distance = particle.Evaluate();
        assert(abs(distance - BestDistance_) < 0.0000001);
        return particle.GetRoutes();
    }
}