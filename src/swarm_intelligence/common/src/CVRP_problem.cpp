#include "CVRP_problem.hpp"

#include <stdexcept>

namespace SI::CVRP {
    // TProblem-------------------------------------------------------------------------
    TProblem::TProblem(const TNodesWithCoordinates& nodes, const TDurationTable& durationMatrix, const TDistanceTable& distanceMatrix)
        : DurationMatrix(durationMatrix)
        , DistanceMatrix(distanceMatrix)
        , Customers_(nodes.Nodes().begin() + 1, nodes.Nodes().end())
        , Depot_(nodes.Nodes()[0])
        , Nodes_(nodes)
    {
        for (const auto& row : durationMatrix) {
            for (const auto& duration : row) {
                if (duration < 0) {
                    throw std::invalid_argument("Duration matrix contains negative values");
                }
            }
        }

        for (const auto& row : distanceMatrix) {
            for (const auto& distance : row) {
                if (distance < 0) {
                    throw std::invalid_argument("Distance matrix contains negative values");
                }
            }
        }
    }

    uint64_t TProblem::NodesCount() const {
        return Customers_.size() + 1;
    }

    uint64_t TProblem::CustomersCount() const {
        return Customers_.size();
    }

    const std::vector<TNode>& TProblem::Customers() const {
        return Customers_;
    }

    const TNode& TProblem::Depot() const {
        return Depot_;
    }

    TCapacity TProblem::Capacity() const {
        return Depot_.Demand;
    }

    const TNodesWithCoordinates& TProblem::Nodes() const {
        return Nodes_;
    }

    TDistance TProblem::Duration(const TNode& from, const TNode& to) const {
        return DurationMatrix[from.ID][to.ID];
    }

    TDistance TProblem::Distance(const TNode& from, const TNode& to) const {
        return DistanceMatrix[from.ID][to.ID];
    }
    // ---------------------------------------------------------------------------------

    // TSolution-------------------------------------------------------------------------
    TSolution::TSolution(const TProblem& problem, const TRoutes& routes, int64_t totalDistance)
        : Problem_(problem)
        , Routes_(routes)
        , TotalDistance_(totalDistance)
    {}

    const TProblem& TSolution::Problem() const {
        return Problem_;
    }

    const TRoutes& TSolution::Routes() const {
        return Routes_;
    }

    int64_t TSolution::TotalDistance() const {
        return TotalDistance_;
    }

    TIteratorRange<TRouteIterator> TSolution::RouteWithCoordinates(const TRoute& route) const {
        return MakeIteratorRange<TRouteIterator>(TRouteIterator(this, &route), TRouteIterator());
    }
    // --------------------------------------------------------------------------------

    // TRouteIterator------------------------------------------------------------------
    TRouteIterator::TRouteIterator(const TSolution* solution, const TRoute* route)
        : Solution_(solution)
        , Route_(route)
        , Index_(0)
    {
        if (Index_ >= Route_->size()) {
            MakeEnd();
        }
    }

    TRouteIterator& TRouteIterator::operator++() {
        ++Index_;
        if (Index_ >= Route_->size()) {
            MakeEnd();
        }
        return *this;
    }

    TNodeWithCoordinate TRouteIterator::operator*() {
        const TNode& node = Route_->at(Index_);
        return {node, Solution_->Problem().Nodes().Coordinates()[node.ID]};
    }

    bool TRouteIterator::operator==(const TRouteIterator& other) {
        return Solution_ == other.Solution_ && Route_ == other.Route_ && Index_ == other.Index_;
    }

    bool TRouteIterator::operator!=(const TRouteIterator& other) {
        return !(*this == other);
    }


    void TRouteIterator::MakeEnd() {
        *this = {};
    }
    // --------------------------------------------------------------------------------

} // namespace SI::CVRP