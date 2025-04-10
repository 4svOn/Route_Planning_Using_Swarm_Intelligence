#include "solver.hpp"

// #include <userver/logging/log.hpp>

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
        // LOG_INFO() << "Solving problem with " << Parameters_.SwarmSize << " particles" ;
        for (int i = 0; i < Parameters_.MaxIterations; ++i) {
            for (auto& particle : Particles_) {
                // TDistance particleDistance =
                particle.Evaluate();

                if (particle.BestDistance() < BestDistance_ || BestPosition_.empty()) {
                    BestDistance_ = particle.BestDistance();
                    BestPosition_ = particle.BestPosition();
                }
            }

            for (auto& particle : Particles_) {
                particle.Update(Parameters_, BestDistance_);
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