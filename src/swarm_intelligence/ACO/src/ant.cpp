#include "ant.hpp"

#include <swarm_intelligence/util/util.hpp>

#include <iostream>

namespace ACO {
    void TEST(const std::string& s) {
        std::cout << s << std::endl;
        std::cout << SI::util::sum(s.size(), 1) << std::endl;
    }
};