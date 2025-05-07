#include "node.hpp"

#include "types.hpp"

namespace SI::CVRP {
    // TNode-----------------------------------------------------------------------------
    TNode::TNode(TNodeID id, TCapacity demand)
        : ID(id)
        , Demand(demand)
    {}

    TNode::TNode(const pb::Customer& customer, int64_t index)
        : ID(index)
        , Demand(customer.demand())
    {}

    bool TNode::operator==(const TNode& other) const {
        return ID == other.ID && Demand == other.Demand;
    }

    bool TNode::operator!=(const TNode& other) const {
        return !(*this == other);
    }

    bool TNode::operator<(const TNode& other) const {
        if (Demand == other.Demand) {
            return ID < other.ID;
        }
        return Demand < other.Demand;
    }
    // ---------------------------------------------------------------------------------

    // TNodeWithCoordinate--------------------------------------------------------------
    TNodeWithCoordinate::TNodeWithCoordinate(const TNode& node, const base::Coordinate& coordinate, const int64_t uid)
        : TNode(node)
        , Coordinate(coordinate)
        , UIDFromFrontend(uid)
    {}
    // ---------------------------------------------------------------------------------

    // TNodesWithCoordinates------------------------------------------------------------
    TNodesWithCoordinates::TNodesWithCoordinates(const std::vector<TNode>& nodes)
        : Nodes_(nodes)
    {
    }

    TNodesWithCoordinates::TNodesWithCoordinates(const pb::Request& request_pb) {
        Nodes_.reserve(request_pb.customers().size() + 1);
        Coordinates_.reserve(request_pb.customers().size() + 1);
        UIDFromFrontend_.reserve(request_pb.customers().size() + 1);
        Nodes_.emplace_back(request_pb.depot(), 0);
        Coordinates_.emplace_back(request_pb.depot().coordinate());
        UIDFromFrontend_.emplace_back(request_pb.depot().uid());
        for (int i = 0; i < request_pb.customers().size(); ++i) {
            Nodes_.emplace_back(request_pb.customers()[i], i + 1);
            Coordinates_.emplace_back(request_pb.customers()[i].coordinate());
            UIDFromFrontend_.emplace_back(request_pb.customers()[i].uid());
        }
    }

    const std::vector<TNode>& TNodesWithCoordinates::Nodes() const {
        return Nodes_;
    }

    const base::Coordinates& TNodesWithCoordinates::Coordinates() const {
        return Coordinates_;
    }

    const std::vector<int64_t>& TNodesWithCoordinates::UIDsFromFrontend() const {
        return UIDFromFrontend_;
    }

    const TNode& TNodesWithCoordinates::Node(int64_t index) const {
        return Nodes_[index];
    }

    const base::Coordinate& TNodesWithCoordinates::Coordinate(int64_t index) const {
        return Coordinates_[index];
    }

    int64_t TNodesWithCoordinates::UIDFromFrontend(int64_t index) const {
        return UIDFromFrontend_[index];
    }
    // ---------------------------------------------------------------------------------
} // namespace SI::CVRP