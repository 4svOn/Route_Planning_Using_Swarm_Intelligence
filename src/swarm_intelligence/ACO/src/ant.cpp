#include "ant.hpp"

#include <base/random.hpp>

# include <swarm_intelligence/common/two_opt.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <ranges>

namespace SI::ACO {
    TAnt::TAnt(const CVRP::TProblem& problem)
        : Problem_(problem)
        , UnvisitedCustomers_(Problem_.Customers().begin(), Problem_.Customers().end())
        , RemainingCapacity_(Problem_.Capacity())
    {
        Routes_.emplace_back();
        Routes_.back().emplace_back(Problem_.Depot());
    }

    void TAnt::Reset() {
        VisitedCustomers_.clear();
        UnvisitedCustomers_.insert(Problem_.Customers().begin(), Problem_.Customers().end());
        RemainingCapacity_ = Problem_.Capacity();
    }

    TDistance TAnt::TotalDistance() const {
        TDistance totalDistance = 0;
        for (const auto& route : Routes_) {
            for (size_t i = 0; i < route.size() - 1; ++i) {
                totalDistance += Problem_.Distance(route[i], route[i + 1]);
            }
        }
        return totalDistance;
    }

    CVRP::TRoutes& TAnt::GetRoutes() {
        return Routes_;
    }

    const CVRP::TRoutes& TAnt::GetRoutes() const {
        return Routes_;
    }

    void TAnt::ConstructSolution(const TParameters& parameters, const TPheromoneMatrix& pheromones) {
        while (!UnvisitedCustomers_.empty()) {
            auto rightBorder = UnvisitedCustomers_.lower_bound(CVRP::TNode::CreateDummy(RemainingCapacity_ + 1));
            auto feasibleCustomers = TSubrange(UnvisitedCustomers_.begin(), rightBorder);
            // auto feasibleCustomers = std::ranges::subrange(UnvisitedCustomers_.lower_bound(CVRP::Node::CreateDummy(RemainingCapacity_)), UnvisitedCustomers_.end());

            if (feasibleCustomers.empty()) {
                ReturnToDepot();
                continue;
            }

            CVRP::TNode nextCustomer = SelectNextCustomer(feasibleCustomers, parameters, pheromones);
            VisitCustomer(nextCustomer);
        }

        if (Routes_.back().back() != Problem_.Depot()) {
            ReturnToDepot();
        }

        Routes_.pop_back();
    }

    void TAnt::MakeTwoOpt() {
        SI::TwoOpt(Problem_, Routes_);
    }


    void TAnt::VisitCustomer(const CVRP::TNode& customer) {
        VisitedCustomers_.emplace(customer);
        UnvisitedCustomers_.erase(customer);
        Routes_.back().emplace_back(customer);
        RemainingCapacity_ -= customer.Demand;
    }

    void TAnt::ReturnToDepot() {
        Routes_.back().emplace_back(Problem_.Depot());
        Routes_.emplace_back();
        Routes_.back().emplace_back(Problem_.Depot());
        RemainingCapacity_ = Problem_.Capacity();
    }

    CVRP::TNode TAnt::SelectNextCustomer(const TSubrange<CVRP::TNodesSet::iterator>& feasibleCustomers, const TParameters& parameters, const TPheromoneMatrix& pheromones) const {
        double sum = 0;
        const auto& curNode = Routes_.back().back();
        std::vector <double> probabilities;
        for (const auto& customer : feasibleCustomers) {
            double q = std::pow(pheromones[curNode.ID][customer.ID], parameters.PheromonceImportance);
            q *= std::pow(1. / static_cast<double>(Problem_.Distance(curNode, customer)), parameters.DistanceImportance);
            // {
            //     double toDepot = static_cast<double>(Problem_.Distance(curNode, Problem_.Depot()));
            //     double fromDepot = static_cast<double>(Problem_.Distance(Problem_.Depot(), customer));
            //     double toCustomer = static_cast<double>(Problem_.Distance(curNode, customer));
            //     double value = toDepot + fromDepot - parameters.G * toCustomer - parameters.F * std::abs(toCustomer - fromDepot);
            //     q *= std::pow(value, parameters.DistanceImportance);
            // }
            probabilities.emplace_back(q);
            sum += q;
        }

        if (RandomUniform() < parameters.ProbabalisticBorder) {
            auto q = *std::next(feasibleCustomers.begin(), std::max_element(probabilities.begin(), probabilities.end()) - probabilities.begin());
            return q;
        }

        std::for_each(probabilities.begin(), probabilities.end(), [&sum](double& p) { p /= sum; });

        auto pos = WeightedRandomSelection(probabilities);
        if (pos == -1) {
            return *feasibleCustomers.begin();
        }
        return *std::next(feasibleCustomers.begin(), pos);
    }
};