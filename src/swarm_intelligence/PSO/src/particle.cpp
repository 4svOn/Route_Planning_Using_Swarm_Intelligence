#include "particle.hpp"

#include <base/random.hpp>

#include <swarm_intelligence/common/two_opt.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace {
    double Similarity(const double a, const double b) {
        // Чем ближе позиция клиента i в a и b, тем выше значение
        return 1.0 / (1.0 + std::abs(a - b));
    }
}

namespace SI::PSO {
    TParticle::TParticle(const CVRP::TProblem& problem)
        : Problem_(problem)
        , Position_(RandomPermutation(problem.CustomersCount()))
        , IterationsWithoutImprovement_(0)
    {
        Evaluate();
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
        return Problem_.RoutesTotalDistance(Routes_);
    }

    const CVRP::TRoutes& TParticle::GetRoutes() const {
        return Routes_;
    }

    TDistance TParticle::Evaluate() {
        Routes_.clear();

        // using TCoord = std::pair<double, CVRP::TNode>;
        // std::vector<TCoord> positions(Problem_.CustomersCount());
        // for (size_t i = 0; i < positions.size(); ++i) {
        //     positions[i] = {Position_[i], Problem_.Customers()[i]};
        // }
        // std::sort(positions.begin(), positions.end());

        Routes_.emplace_back();
        Routes_.back().emplace_back(Problem_.Depot());
        TCapacity currentCapacity = Problem_.Capacity();
        for (const auto& nodeID : Position_) {
            const auto& node = Problem_.Customers()[nodeID];
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
            IterationsWithoutImprovement_ = 0;
        } else {
            IterationsWithoutImprovement_++;
        }

        return TotalDistance();
    }

    TDistance TParticle::BestDistance() const {
        return BestDistance_;
    }

    const TCoordinates& TParticle::BestPosition() const {
        return BestPosition_;
    }

    void TParticle::MakeTwoOpt() {
        BestDistance_ = TParticle::MakeTwoOpt(Problem_, BestPosition_, BestDistance_);
    }

    void TParticle::Update(const TParameters& parameters, const TCoordinates& bestGlobalPosition, bool isDebugPrint) {
        if (IterationsWithoutImprovement_ >= 100) {
            Position_ = RandomPermutation(Position_.size());
            Evaluate();
            IterationsWithoutImprovement_ = 0;
        }

        // for (size_t i = 0; i < Position_.size(); ++i) {
            auto rnd = RandomUniform();
            if (rnd < parameters.W) {
                size_t i = RandomUniform(0, Position_.size() - 1);
                size_t j = RandomUniform(i + 1 , Position_.size());
                std::shuffle(Position_.begin() + i, Position_.begin() + j, RandomGenerator());
                return;
            }

            const auto& source = (rnd < parameters.W + parameters.C1) ? BestPosition_ : bestGlobalPosition;

            // [start, end)
            size_t start = RandomUniform(0, source.size() - 1);
            size_t end = start + 1 + RandomUniform(0, source.size() - 1 - start);

            InsertSegment(source, start, end);

        // }

        // Repair();

        // if (RandomUniform() < parameters.M) {
        //     size_t i = RandomUniform(0, Position_.size() - 1);
        //     size_t j = RandomUniform(i + 1 , Position_.size());
        //     std::shuffle(Position_.begin() + i, Position_.begin() + j, RandomGenerator());
        // }

        assert(Position_.size() == Problem_.CustomersCount());

        if (isDebugPrint) {
            DebugPrint(std::cout);
        }
    }

    void TParticle::InsertSegment(const TCoordinates& source, size_t start, size_t end) {
        // Копируем сегмент
        assert(end <= source.size());
        TCoordinates segment(source.begin() + start, source.begin() + end);

        // Удаляем клиенты из сегмента из текущей позиции
        for (auto customer : segment) {
            auto it = std::find(Position_.begin(), Position_.end(), customer);
            if (it != Position_.end()) {
                Position_.erase(it);
            }
        }

        // Вставляем сегмент в случайную позицию
        size_t insert_pos = RandomUniform(0, Position_.size() - 1);
        Position_.insert(Position_.begin() + insert_pos, segment.begin(), segment.end());
    }

    void TParticle::Repair() {
        std::unordered_set<TCoordinate> unique;
        TCoordinates repaired;

        // Удаление дубликатов
        for (auto customer : Position_) {
            if (unique.insert(customer).second) {
                repaired.push_back(customer);
            }
        }

        // Добавление пропущенных клиентов
        for (int64_t customer = 0; customer < Problem_.CustomersCount(); ++customer) {
            if (!unique.count(customer)) {
                repaired.push_back(customer);
            }
        }

        if (Position_ != repaired) {
            std::cout << "Repairing particle" << std::endl;
            std::cout << "Old position: " << std::endl;
            for (auto customer : Position_) {
                std::cout << customer << " ";
            }
            std::cout << std::endl;
            std::cout << "New position: " << std::endl;
            for (auto customer : repaired) {
                std::cout << customer << " ";
            }
            std::cout << std::endl;
            exit(0);
        }

        Position_ = repaired;
    }

    void TParticle::DebugPrint(std::ostream& stream) {
        stream << "Particle: " << std::endl;
        stream << "Position: " << std::endl;
        for (size_t i = 0; i < Position_.size(); ++i) {
            stream << Position_[i] << " ";
        }
        stream << std::endl;
        stream << "Velocity: " << std::endl;
        for (size_t i = 0; i < Velocity_.size(); ++i) {
            stream << Velocity_[i] << " ";
        }
        stream << std::endl;
        stream << "Best Position: " << std::endl;
        for (size_t i = 0; i < BestPosition_.size(); ++i) {
            stream << BestPosition_[i] << " ";
        }
        stream << std::endl;
        stream << "Best Distance: " << BestDistance_ << std::endl;
        stream << std::endl;
    }

    TDistance TParticle::MakeTwoOpt(const CVRP::TProblem& problem, TCoordinates& position, TDistance oldBestDistance) {
        TParticle particle{problem, position};

        CVRP::TRoutes routes = particle.GetRoutes();
        auto logs = SI::TwoOpt(problem, routes);
        TDistance newTotalDistance = problem.RoutesTotalDistance(routes);
        if (newTotalDistance >= oldBestDistance) {
            return oldBestDistance;
        }

        int64_t i = 0;
        for (const auto& route : routes) {
            for (const auto& node : route) {
                if (node == problem.Depot()) {
                    continue;
                }
                assert(i < position.size());
                position[i] = node.ID - 1;
                i++;
            }
        }
        assert(i == position.size());

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