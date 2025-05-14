#include "particle.hpp"

#include <base/random.hpp>

#include <swarm_intelligence/common/two_opt.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <unordered_set>

namespace {
    double Similarity(const double a, const double b) {
        // Чем ближе позиция клиента i в a и b, тем выше значение
        return 1.0 / (1.0 + std::abs(a - b));
    }
}

namespace SI::PSO {
    TParticle::TParticle(const CVRP::TProblem& problem)
        : Problem_(problem)
        , IterationsWithoutImprovement_(0)
    {
        Position_.resize(problem.NodesCount(), std::vector<double>(problem.NodesCount(), 0.0));
        for (size_t i = 0; i < problem.NodesCount(); ++i) {
            double totalLength = 0.0;
            for (size_t j = 0; j < problem.NodesCount(); ++j) {
                if (i == j) {
                    continue;
                }
                totalLength += Problem_.Distance(Problem_.Nodes().Nodes()[i], Problem_.Nodes().Nodes()[j]);
            }
            for (size_t j = 0; j < problem.NodesCount(); ++j) {
                Position_[i][j] = Problem_.Distance(Problem_.Nodes().Nodes()[i], Problem_.Nodes().Nodes()[j]) / totalLength;
            }
        }
        Evaluate();
        UpdateBestPosition();
        // Position_.resize(problem.CustomersCount());
        // Velocity_.resize(problem.CustomersCount());

        // for (size_t i = 0; i < problem.CustomersCount(); ++i) {
            // Position_[i] = {RandomUniform(), Problem_.Customers()[i]};
            // Position_[i] = RandomUniform();
            // Velocity_[i] = {0.0, Problem_.Customers()[i]};
            // Velocity_[i] = 0.0;
        // }
    }

    TParticle::TParticle(const CVRP::TProblem& problem, const TCoordinates& position)
        : Problem_(problem)
        , Position_(position)
    {
        Evaluate();
    }

    TDistance TParticle::TotalDistance() const {
        return TotalDistance_;
    }

    const CVRP::TRoutes& TParticle::GetRoutes() const {
        return Routes_;
    }

    TDistance TParticle::Evaluate() {
        auto position = Position_;
        std::unordered_set<int64_t> visited;
        visited.insert(0); // depot
        auto isVisited = [&visited](int64_t node) {
            return visited.find(node) != visited.end();
        };

        Routes_.clear();
        Routes_.emplace_back();
        Routes_.back().emplace_back(Problem_.Depot());
        int64_t currentNode = 0;
        int64_t remainingCapacity = Problem_.Capacity();

        auto returnToDepot = [&]() {
            Routes_.back().emplace_back(Problem_.Depot());
            Routes_.emplace_back();
            Routes_.back().emplace_back(Problem_.Depot());
            remainingCapacity = Problem_.Capacity();
        };

        while (visited.size() < Problem_.CustomersCount()) {
            int64_t nextNode = -1;

            auto probabilities = Position_[currentNode];

            double sum = 0.0;
            for (size_t i = 0; i < probabilities.size(); ++i) {
                if (!isVisited(i) && Problem_.Nodes().Nodes()[i].Demand <= remainingCapacity) {
                    sum += probabilities[i];
                } else {
                    probabilities[i] = 0.0;
                }
            }
            if (sum <= TParameters::EPS) {
                returnToDepot();
                continue;
            }

            for (size_t i = 0; i < probabilities.size(); ++i) {
                probabilities[i] /= sum;
            }
            auto pos = WeightedRandomSelection(probabilities);
            if (pos == -1) {
                pos = 0;
            }

            auto node = Problem_.Nodes().Nodes()[pos];
            Routes_.back().emplace_back(node);
            visited.insert(pos);
            remainingCapacity -= node.Demand;
            currentNode = pos;
        }

        if (Routes_.back().back() != Problem_.Depot()) {
            Routes_.back().emplace_back(Problem_.Depot());
        }

        TotalDistance_ = Problem_.RoutesTotalDistance(Routes_);

        return TotalDistance();
    }

    TDistance TParticle::BestDistance() const {
        return BestDistance_;
    }

    const TCoordinates& TParticle::BestPosition() const {
        return BestPosition_;
    }

    const CVRP::TRoutes& TParticle::BestRoutes() const {
        return BestRoutes_;
    }

    void TParticle::MakeTwoOpt() {
        BestDistance_ = TParticle::MakeTwoOpt(Problem_, BestRoutes_, BestDistance_);
    }

    void TParticle::UpdateBestPosition() {
        TDistance totalDistance = TotalDistance();
        if (totalDistance < BestDistance_ || BestPosition_.empty()) {
            BestDistance_ = totalDistance;
            BestPosition_ = Position_;
            BestRoutes_ = Routes_;
            IterationsWithoutImprovement_ = 0;
        } else {
            IterationsWithoutImprovement_++;
        }
    }

    void TParticle::Update(const TParameters& parameters, const TCoordinates& bestGlobalPosition, bool isDebugPrint) {
        for (size_t i = 0; i < Position_.size(); ++i) {
            for (size_t j = 0; j < Position_[i].size(); ++j) {
                Position_[i][j] = parameters.W * Position_[i][j] + parameters.C1 * bestGlobalPosition[i][j] + parameters.C2 * BestPosition_[i][j];
            }
        }

        UpdateBestPosition();
    }

    // void TParticle::InsertSegment(const TCoordinates& source, size_t start, size_t end) {
    //     // Копируем сегмент
    //     assert(end <= source.size());
    //     TCoordinates segment(source.begin() + start, source.begin() + end);

    //     // Удаляем клиенты из сегмента из текущей позиции
    //     for (auto customer : segment) {
    //         auto it = std::find(Position_.begin(), Position_.end(), customer);
    //         if (it != Position_.end()) {
    //             Position_.erase(it);
    //         }
    //     }

    //     // Вставляем сегмент в случайную позицию
    //     size_t insert_pos = RandomUniform(0, Position_.size() - 1);
    //     Position_.insert(Position_.begin() + insert_pos, segment.begin(), segment.end());
    // }

    // void TParticle::Repair() {
    //     std::unordered_set<TCoordinate> unique;
    //     TCoordinates repaired;

    //     // Удаление дубликатов
    //     for (auto customer : Position_) {
    //         if (unique.insert(customer).second) {
    //             repaired.push_back(customer);
    //         }
    //     }

    //     // Добавление пропущенных клиентов
    //     for (int64_t customer = 0; customer < Problem_.CustomersCount(); ++customer) {
    //         if (!unique.count(customer)) {
    //             repaired.push_back(customer);
    //         }
    //     }

    //     if (Position_ != repaired) {
    //         std::cout << "Repairing particle" << std::endl;
    //         std::cout << "Old position: " << std::endl;
    //         for (auto customer : Position_) {
    //             std::cout << customer << " ";
    //         }
    //         std::cout << std::endl;
    //         std::cout << "New position: " << std::endl;
    //         for (auto customer : repaired) {
    //             std::cout << customer << " ";
    //         }
    //         std::cout << std::endl;
    //         exit(0);
    //     }

    //     Position_ = repaired;
    // }

    // void TParticle::DebugPrint(std::ostream& stream) {
    //     stream << "Particle: " << std::endl;
    //     stream << "Position: " << std::endl;
    //     for (size_t i = 0; i < Position_.size(); ++i) {
    //         stream << Position_[i] << " ";
    //     }
    //     stream << std::endl;
    //     stream << "Velocity: " << std::endl;
    //     for (size_t i = 0; i < Velocity_.size(); ++i) {
    //         stream << Velocity_[i] << " ";
    //     }
    //     stream << std::endl;
    //     stream << "Best Position: " << std::endl;
    //     for (size_t i = 0; i < BestPosition_.size(); ++i) {
    //         stream << BestPosition_[i] << " ";
    //     }
    //     stream << std::endl;
    //     stream << "Best Distance: " << BestDistance_ << std::endl;
    //     stream << std::endl;
    // }

    TDistance TParticle::MakeTwoOpt(const CVRP::TProblem& problem, CVRP::TRoutes& routes, TDistance oldBestDistance) {
        auto logs = SI::TwoOpt(problem, routes);
        TDistance newTotalDistance = problem.RoutesTotalDistance(routes);
        if (newTotalDistance >= oldBestDistance) {
            return oldBestDistance;
        }
        return newTotalDistance;
    }
} // namespace SI::PSO

        // for (size_t i = 0; i < Velocity_.size(); ++i) {
        //     Velocity_[i] = parameters.W * Velocity_[i]
        //                    + parameters.C1 * RandomUniform() * Similarity(BestPosition_[i], Position_[i])
        //                    + parameters.C2 * RandomUniform() * Similarity(bestGlobalPosition[i], Position_[i]);
        //     Velocity_[i] =  1.0 / (1.0 + std::exp(-Velocity_[i]));
        // }

        // for (size_t i = 0; i < Position_.size(); ++i) {
        //     if (RandomUniform() < Velocity_[i]) {
        //         double alpha = 0.5 * (BestPosition_[i] + bestGlobalPosition[i]);
        //         double randomAdjustment = RandomUniform() * 0.2 - 0.1; // [0, 1] -> [-0.1, 0.1]
        //         Position_[i] = alpha + randomAdjustment;
        //     }
        // }

        // auto sumVelocity = *std::max_element(Velocity_.begin(), Velocity_.end());
        // for (size_t i = 0; i < Velocity_.size(); ++i) {
        //     Velocity_[i] /= sumVelocity;
        // }

        // auto sum = *std::max_element(Position_.begin(), Position_.end());
        // for (size_t i = 0; i < Position_.size(); ++i) {
        //     Position_[i] /= sum;
        // }