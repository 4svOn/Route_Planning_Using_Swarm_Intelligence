#include "random.hpp"

#include <algorithm>
#include <random>

namespace {
    class Random {
    public:
        Random(unsigned seed) : Generator_(seed) {}

        int32_t WeightedRandomSelection(const std::vector<double>& probabilities) {
            std::vector<double> cumulative;
            std::partial_sum(probabilities.begin(), probabilities.end(), std::back_inserter(cumulative));

            double randomValue = Distribution_(Generator_);

            auto it = std::lower_bound(cumulative.begin(), cumulative.end(), randomValue);
            if (it != cumulative.end()) {
                return std::distance(cumulative.begin(), it);
            }

            return -1;
        }

        double RandomUniform() {
            return Distribution_(Generator_);
        }

    private:
        std::mt19937 Generator_;
        std::uniform_real_distribution<> Distribution_{0.0, 1.0};
    };

    // static Random random(std::random_device{}());
    static Random RANDOM(228228227);
}

int32_t WeightedRandomSelection(const std::vector<double>& probabilities) {
    return RANDOM.WeightedRandomSelection(probabilities);
}

double RandomUniform() {
    return RANDOM.RandomUniform();
}
