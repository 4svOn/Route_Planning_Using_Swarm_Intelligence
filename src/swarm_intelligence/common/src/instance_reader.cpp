#include "instance_reader.hpp"

#include <cassert>
#include <exception>
#include <iostream>
#include <optional>
#include <regex>
#include <cmath>

namespace {
    const static std::string NODE_SECTION_START = "NODE_COORD_SECTION";
    const static std::string DEMAND_SECTION_START = "DEMAND_SECTION";
    const static std::string DEPOT_SECTION_START = "DEPOT_SECTION";
    const static std::string END_OF_FILE = "EOF";
    const static std::string NAME = "NAME : ";
    const static std::string COMMENT = "COMMENT : ";
    const static std::string DIMMENSION = "DIMENSION : ";
    const static std::string EDGE_WEIGHT_TYPE  = "EDGE_WEIGHT_TYPE : ";
    const static std::string CAPACITY  = "CAPACITY : ";

    // Функция для удаления пробелов в начале и конце строки
    std::string trim(const std::string &s) {
        auto start = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
        auto end = std::find_if_not(s.rbegin(), s.rend(), [](int c) { return std::isspace(c); }).base();
        return (start < end) ? std::string(start, end) : "";
    }

    std::vector<std::string> split(const std::string &S) {
        std::string s = trim(S);
        std::regex re("\\s+");  // разделитель — один или больше пробелов
        std::sregex_token_iterator it(s.begin(), s.end(), re, -1);
        std::sregex_token_iterator end;
        return {it, end};
    }
}

namespace SI::CVRP {
    TFileInstanceReader::TFileInstanceReader(const std::string& file_name)
        : File_(file_name)
    {
        if (!File_.is_open()) {
            throw std::runtime_error("Failed to open file: " + file_name);
        }
    }

    TFileInstanceReader::~TFileInstanceReader() {
        File_.close();
    }

    TProblem TFileInstanceReader::Read() {
        std::string line;
        State_ = State::READ_BASE_INFO;
        while (std::getline(File_, line)) {
            switch(State_) {
                case State::READ_BASE_INFO:
                    State_ = ReadBaseInfo(line);
                    break;
                case State::READ_NODES:
                    State_ = ReadNodes(line);
                    break;
                case State::READ_DEMANDS:
                    State_ = ReadDemands(line);
                    break;
                case State::READ_DEPOT:
                    State_ = ReadDepot(line);
                    break;
                case State::DONE:
                    break;
            }
        }

        Nodes[DepotIndex].Demand = Capacity;
        if (DepotIndex != 0) {
            std::swap(Nodes[DepotIndex], Nodes[0]);
            DepotIndex = 0;
        }
        auto distanceTable = CalcDistanceTable();
        TNodesWithCoordinates nodesWithCoordinates{Nodes};
        // for (const auto& node : nodesWithCoordinates.Nodes()) {
        //     std::cout << node.ID << " ";
        // }
        // std::cout << std::endl;
        return {nodesWithCoordinates, {}, distanceTable};
    }

    TFileInstanceReader::State TFileInstanceReader::ReadBaseInfo(const std::string& line) {
        if (line.find(NODE_SECTION_START) != std::string::npos) {
            return State::READ_NODES;
        }

        auto extract = [&line](const std::string& token) -> std::optional<std::string> {
            if (const auto pos = line.find(token); pos != std::string::npos) {
                return trim(line.substr(pos + token.length()));
            }
            return {};
        };

        if (const auto str = extract(NAME)) {
            Name = *str;
        }

        if (const auto str = extract(COMMENT)) {
            Comment = *str;
        }

        if (const auto str = extract(DIMMENSION)) {
            Dimension = std::stoll(*str);
        }

        if (const auto str = extract(EDGE_WEIGHT_TYPE)) {
            if (*str == "EUC_2D") {
                EdgeWeightType_ = EdgeWeightType::EUC_2D;
            } else if (*str == "MANHATTAN") {
                EdgeWeightType_ = EdgeWeightType::MANHATTAN;
            } else {
                throw std::runtime_error("Unknown edge weight type: " + *str);
            }
        }

        if (const auto str = extract(CAPACITY)) {
            Capacity = std::stoll(*str);
        }

        return State::READ_BASE_INFO;
    }

    TFileInstanceReader::State TFileInstanceReader::ReadNodes(const std::string& line) {
        if (line.find(DEMAND_SECTION_START) != std::string::npos) {
            return State::READ_DEMANDS;
        }

        auto tokens = split(line);
        assert(tokens.size() == 3);

        Nodes.emplace_back(std::stoll(tokens[0]) - 1, -1);
        NodesCoordinates_.emplace_back(std::stoll(tokens[1]), std::stoll(tokens[2]));

        return State::READ_NODES;
    }

    TFileInstanceReader::State TFileInstanceReader::ReadDemands(const std::string& line) {
        if (line.find(DEPOT_SECTION_START) != std::string::npos) {
            return State::READ_DEPOT;
        }

        auto tokens = split(line);
        assert(tokens.size() == 2);

        auto index = std::stoll(tokens[0]) - 1;
        assert(index < Nodes.size());

        Nodes[index].Demand = std::stoll(tokens[1]);

        return State::READ_DEMANDS;
    }

    TFileInstanceReader::State TFileInstanceReader::ReadDepot(const std::string& line) {
        if (line.find(END_OF_FILE) != std::string::npos) {
            return State::DONE;
        }

        auto tokens = split(line);
        assert(tokens.size() == 1);

        auto index = std::stoll(tokens[0]) - 1;
        if (index >= 0) {
            DepotIndex = std::stoll(tokens[0]) - 1;
        }

        return State::READ_DEPOT;
    }

    TDistanceTable TFileInstanceReader::CalcDistanceTable() {
        TDistanceTable result(Dimension, std::vector<TDistance>(Dimension, 0));
        assert(NodesCoordinates_.size() == Dimension);

        for (int i = 0; i < Dimension; i++) {
            for (int j = 0; j < Dimension; j++) {
                if (i == j) {
                    continue;
                }
                Coord nodeI = NodesCoordinates_[i];
                Coord nodeJ = NodesCoordinates_[j];

                result[i][j] = CalcDistance(nodeI, nodeJ);
            }
        }

        return result;
    }

    TDistance TFileInstanceReader::CalcDistance(const Coord& node1, const Coord& node2) {
        switch (EdgeWeightType_) {
            case EdgeWeightType::EUC_2D:
                return std::ceil(std::sqrt(std::pow(node1.first - node2.first, 2) + std::pow(node1.second - node2.second, 2)) * MULTIPLIER);
            case EdgeWeightType::MANHATTAN:
                return std::abs(node1.first - node2.first) + std::abs(node1.second - node2.second);
            default:
                throw std::runtime_error("Unknown edge weight type");
        }
    }
}