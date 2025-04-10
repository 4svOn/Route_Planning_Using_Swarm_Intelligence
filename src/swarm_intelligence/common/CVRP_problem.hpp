#pragma once

#include "node.hpp"
#include "types.hpp"

#include "base/ranges.hpp"

#include <cstdint>
#include <vector>
#include <set>

// namespace std {
//     template<>
//     struct hash<SI::CVRP::Node> {
//         size_t operator()(const SI::CVRP::Node& node) const {
//             // Используем метод объединения хешей, аналогичный boost::hash_combine
//             size_t seed = 0;
//             const size_t prime = 0x9e3779b9;

//             // Комбинируем хеш для поля 'ID'
//             seed ^= hash<SI::NodeID>{}(node.ID) + prime + (seed << 6) + (seed >> 2);

//             // Комбинируем хеш для поля 'Demand'
//             seed ^= hash<std::int64_t>{}(node.Demand) + prime + (seed << 6) + (seed >> 2);

//             return seed;
//         }
//     };
// } // namespace std

namespace SI::CVRP {
    class TProblem {
    public:
        TProblem(const TNodesWithCoordinates& nodes, const TDurationTable& durationMatrix, const TDistanceTable& distanceMatrix);

        uint64_t NodesCount() const;
        uint64_t CustomersCount() const;

        const std::vector<TNode>& Customers() const;
        const TNode& Depot() const;
        TCapacity Capacity() const;

        const TNodesWithCoordinates& Nodes() const;

        TDuration Duration(const TNode& from, const TNode& to) const;
        TDistance Distance(const TNode& from, const TNode& to) const;

    private:
        TNode Depot_;
        std::vector<TNode> Customers_;

        // 0 index is depot
        TDurationTable DurationMatrix;
        TDistanceTable DistanceMatrix;

        const TNodesWithCoordinates& Nodes_;
    };

    class TRouteIterator;

    class TSolution {
    public:
        TSolution(const TProblem& problem, const TRoutes& routes, int64_t totalDistance);

        TSolution& operator=(const TSolution& other) = default;

        const TProblem& Problem() const;
        const TRoutes& Routes() const;
        int64_t TotalDistance() const;

        TIteratorRange<TRouteIterator> RouteWithCoordinates(const TRoute& route) const;
        TIteratorRange<TRouteIterator> RouteWithCoordinates(const int64_t index) const;

        bool operator<(const TSolution& other) const;

    private:
        const TProblem& Problem_;
        TRoutes Routes_;
        int64_t TotalDistance_;
    };

    class TRouteIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = TNodeWithCoordinate;
        using difference_type = std::ptrdiff_t;
        using pointer = TNodeWithCoordinate*;
        using reference = TNodeWithCoordinate&;

        TRouteIterator() = default;
        TRouteIterator(const TSolution* solution, const TRoute* route);

        TRouteIterator& operator++();
        TNodeWithCoordinate operator*();
        bool operator==(const TRouteIterator& other);
        bool operator!=(const TRouteIterator& other);

    private:
        void MakeEnd();

    private:
        const TSolution* Solution_{nullptr};
        const TRoute* Route_{nullptr};
        int64_t Index_{-1};
    };

} // namespace SI::CVRP