# include "solver.hpp"

# include <cassert>

namespace SI::ACO {
    TSolver::TSolver(const CVRP::TProblem& problem, const TParameters& parameters)
        : Problem_(problem)
        , Parameters_(parameters)
        , Pheromones_(Problem_.NodesCount(), std::vector<double>(Problem_.NodesCount(), 1.0))
    {
        InitializePheromones();
    }

    CVRP::TSolution TSolver::Solve() {
        for (int64_t i = 0; i < Parameters_.MaxIterations; ++i) {
            for (int64_t j = 0; j < Parameters_.AntsPerIteration; ++j) {
                Ants_.emplace_back(Problem_);
                Ants_.back().ConstructSolution(Parameters_, Pheromones_);

                if (Ants_.back().TotalDistance() < BestDistance_) {
                    BestDistance_ = Ants_.back().TotalDistance();
                    BestAntIndex_ = Ants_.size() - 1;
                }
                EvaporatePheromones(Ants_.back());
            }
            DepositPheromones(Ants_[BestAntIndex_]);
        }

        return {Problem_, Ants_[BestAntIndex_].GetRoutes(), BestDistance_};

    }

    TDistance TSolver::GetBestDistance() const {
        return BestDistance_;
    }

    void TSolver::InitializePheromones() {
        TParameters params = Parameters_;
        params.PheromonceImportance = 0.0;
        params.DistanceImportance = 1.0;
        params.ProbabalisticBorder = -1.0;

        Ants_.emplace_back(Problem_);
        Ants_.back().ConstructSolution(params, Pheromones_);
        BestAntIndex_ = 0;

        BestDistance_ = Ants_.back().TotalDistance();
        BasePheromone_ = 1 / (BestDistance_ * Problem_.CustomersCount());
        Pheromones_.assign(Problem_.NodesCount(), std::vector<double>(Problem_.NodesCount(), BasePheromone_));
    }

    // local update
    void TSolver::EvaporatePheromones(const TAnt& ant) {
        for (const auto& route : ant.GetRoutes()) {
            for (size_t i = 0; i < route.size() - 1; ++i) {
                Pheromones_[route[i].ID][route[i + 1].ID] *= (1 - Parameters_.EvaporateRate);
                Pheromones_[route[i].ID][route[i + 1].ID] += Parameters_.EvaporateRate * BasePheromone_;
            }
        }
    }

    // global update
    void TSolver::DepositPheromones(const TAnt& ant) {
        double delta = Parameters_.EvaporateRate / ant.TotalDistance();
        for (const auto& route : ant.GetRoutes()) {
            for (size_t i = 0; i < route.size() - 1; ++i) {
                Pheromones_[route[i].ID][route[i + 1].ID] *= (1 - Parameters_.EvaporateRate);
                Pheromones_[route[i].ID][route[i + 1].ID] += delta;
            }
        }
    }
}
