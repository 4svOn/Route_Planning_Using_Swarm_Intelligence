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
        // LOG_INFO() << "Solving problem with " << Parameters_.SwarmSize << " particles" ;
        int64_t countSameResult = 0;
        for (int i = 0; i < Parameters_.MaxIterations; ++i) {
            for (auto& particle : Particles_) {
                particle.Evaluate();

                if (RandomUniform() < 0.1) {
                    particle.TwoOpt();
                }

                if (particle.BestDistance() < BestDistance_ || BestPosition_.empty()) {
                    BestDistance_ = particle.BestDistance();
                    BestPosition_ = particle.BestPosition();
                    countSameResult = 0;
                } else {
                    countSameResult++;
                }
            }

            // if (i % 10 == 0) {
                TwoOpt();
                // std::cout << "aospapoambpo\n";
            // }

            if (i % 100 == 0) {
                // std::cout << "Iteration: " << i << " Best distance: " << BestDistance_ << std::endl;
                // std::cout << "Best distance: " << Particles_[10].BestDistance() << std::endl;
                // std::cout << "W: " << Parameters_.W << " C1: " << Parameters_.C1 << " C2: " << Parameters_.C2 << " M: " << Parameters_.M << std::endl;
                // std::cout <<  Parameters_.MAX_W - (Parameters_.MAX_W - Parameters_.MIN_W) << " " << (static_cast<double>(i) / static_cast<double>(Parameters_.MaxIterations)) << std::endl;
            }
            bool f = 1;
            for (auto& particle : Particles_) {
                particle.Update(Parameters_, BestPosition_);
                f = 0;
            }

            // Parameters_.W = Parameters_.MAX_W - (Parameters_.MAX_W - Parameters_.MIN_W)
            //     * (static_cast<double>(i) / static_cast<double>(Parameters_.MaxIterations));
            // Parameters_.C1 = (1 - Parameters_.W) / 2;
            // Parameters_.C2 = 1 - Parameters_.W - Parameters_.C1;
            // Parameters_.M = Parameters_.W - 0.2;


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

    void TSolver::TwoOpt() {
        BestDistance_ = TParticle::TwoOpt(Problem_, BestPosition_, BestDistance_);
    }
}