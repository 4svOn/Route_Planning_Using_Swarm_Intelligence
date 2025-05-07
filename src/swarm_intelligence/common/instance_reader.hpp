#pragma once

#include "CVRP_problem.hpp"
#include "node.hpp"
#include "types.hpp"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace SI::CVRP {

    class TFileInstanceReader {
    private:
        enum class State {
            INIT,
            READ_BASE_INFO,
            READ_NODES,
            READ_DEMANDS,
            READ_DEPOT,
            DONE
        };

        enum class EdgeWeightType {
            NONE,
            EUC_2D,
            MANHATTAN
        };

        State State_ = State::INIT;
        EdgeWeightType EdgeWeightType_ = EdgeWeightType::NONE;

        using Coord = std::pair<int64_t, int64_t>;

    public:
        const static int64_t MULTIPLIER = 100;

        TFileInstanceReader(const std::string& file_name);
        ~TFileInstanceReader();

        TProblem Read();

    private:
        State ReadBaseInfo(const std::string& line);
        State ReadNodes(const std::string& line);
        State ReadDemands(const std::string& line);
        State ReadDepot(const std::string& line);

        TDistanceTable CalcDistanceTable();
        TDistance CalcDistance(const Coord& node1, const Coord& node2);

    private:
        std::ifstream File_;

        std::vector<Coord> NodesCoordinates_;

    public:
        std::string Name;
        std::string Comment;
        int64_t Dimension;
        int64_t Capacity;
        std::vector<TNode> Nodes;
        int64_t DepotIndex;
    };

}