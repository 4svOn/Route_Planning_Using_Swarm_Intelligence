#include "particle.hpp"

#include <base/random.hpp>

#include <algorithm>

namespace SI::PSO {
    TParticle::TParticle(const CVRP::TProblem& problem)
        : Problem_(problem)
    {
        Position_.resize(problem.CustomersCount());
        Velocity_.resize(problem.CustomersCount());

        for (size_t i = 0; i < problem.CustomersCount(); ++i) {
            // Position_[i] = {RandomUniform(), Problem_.Customers()[i]};
            Position_[i] = RandomUniform();
            // Velocity_[i] = {0.0, Problem_.Customers()[i]};
            Velocity_[i] = 0.0;
        }
    }

    TParticle::TParticle(const CVRP::TProblem& problem, const std::vector<TCoordinate>& position)
        : Problem_(problem)
        , Position_(position)
    {
    }

    TDistance TParticle::TotalDistance() const {
        TDistance totalDistance = 0;
        for (const auto& route : Routes_) {
            for (size_t i = 0; i < route.size() - 1; ++i) {
                totalDistance += Problem_.Distance(route[i], route[i + 1]);
            }
        }
        return totalDistance;
    }

    const CVRP::TRoutes& TParticle::GetRoutes() const {
        return Routes_;
    }

    TDistance TParticle::Evaluate() {
        Routes_.clear();

        using TCoord = std::pair<double, CVRP::TNode>;
        std::vector<TCoord> positions(Problem_.CustomersCount());
        for (size_t i = 0; i < positions.size(); ++i) {
            positions[i] = {Position_[i], Problem_.Customers()[i]};
        }
        std::sort(positions.begin(), positions.end());

        Routes_.emplace_back();
        Routes_.back().emplace_back(Problem_.Depot());
        TCapacity currentCapacity = Problem_.Capacity();
        for (const auto& [key, node] : positions) {
            if (currentCapacity < node.Demand) {
                Routes_.back().emplace_back(Problem_.Depot());
                Routes_.emplace_back();
                Routes_.back().emplace_back(Problem_.Depot());
                currentCapacity = Problem_.Capacity();
            }
            Routes_.back().emplace_back(node);
            currentCapacity -= node.Demand;
        }

        if (Routes_.back().back() != Problem_.Depot()) {
            Routes_.back().emplace_back(Problem_.Depot());
        }

        TDistance totalDistance = TotalDistance();
        if (totalDistance < BestDistance_ || BestPosition_.empty()) {
            BestDistance_ = totalDistance;
            BestPosition_ = Position_;
        }

        return TotalDistance();
    }

    TDistance TParticle::BestDistance() const {
        return BestDistance_;
    }

    const std::vector<TCoordinate>& TParticle::BestPosition() const {
        return BestPosition_;
    }

    void TParticle::Update(const TParameters& parameters, TDistance bestGlobalDistance) {
        for (size_t i = 0; i < Velocity_.size(); ++i) {
            Velocity_[i] = parameters.W * Velocity_[i]
                           + parameters.C1 * RandomUniform() * (BestPosition_[i] - Position_[i])
                           + parameters.C2 * RandomUniform() * (bestGlobalDistance - BestDistance_);

            Position_[i] += Velocity_[i];
            Position_[i] = std::max(0.0, std::min(1.0, Position_[i]));
        }
    }
} // namespace SI::PSO