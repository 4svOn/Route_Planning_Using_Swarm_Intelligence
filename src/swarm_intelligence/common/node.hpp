#pragma once

#include "types.hpp"

#include "base/coordinates.hpp"

#include "proto/cpp/request.pb.h"

#include <cstdint>
#include <set>

namespace SI::CVRP {

    struct TNode {
        TNodeID ID;
        TCapacity Demand;

        TNode() = default;
        TNode(TNodeID id, TCapacity demand);
        TNode(const pb::Customer& customer, int64_t index);

        bool operator==(const TNode& other) const;
        bool operator!=(const TNode& other) const;
        bool operator<(const TNode& other) const;

        static TNode CreateDummy(int64_t demand) {
            return TNode{0, demand};
        }
    };

    using TRoute = std::vector<CVRP::TNode>;
    using TRoutes = std::vector<TRoute>;
    using TNodesSet = std::set<CVRP::TNode>;

    struct TNodeWithCoordinate : TNode {
        base::Coordinate Coordinate;
        int64_t UIDFromFrontend;

        TNodeWithCoordinate(const TNode& node, const base::Coordinate& coordinate, const int64_t uid);
    };

    class TNodesWithCoordinates {
    public:
        TNodesWithCoordinates(const std::vector<TNode>& nodes);
        TNodesWithCoordinates(const pb::Request& request_pb);

        const std::vector<TNode>& Nodes() const;
        const base::Coordinates& Coordinates() const;
        const std::vector<int64_t>& UIDsFromFrontend() const;

        const TNode& Node(int64_t index) const;
        const base::Coordinate& Coordinate(int64_t index) const; // TODO сделать так, чтобы возвращало TNodeWithCoordinate
        int64_t UIDFromFrontend(int64_t index) const;

    private:
        std::vector<TNode> Nodes_;
        base::Coordinates Coordinates_;
        std::vector<int64_t> UIDsFromFrontend_;

    };

} // namespace SI::CVRP