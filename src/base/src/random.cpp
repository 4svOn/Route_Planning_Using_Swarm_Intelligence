#include "random.hpp"

#include <algorithm>
#include <random>
#include <vector>

namespace {
    class Random {
    public:
        Random(unsigned seed, double min = 0.0, double max = 1.0) : Generator_(seed), Distribution_(min, max) {}

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

        std::mt19937& Generator() { return Generator_; }

    private:
        std::mt19937 Generator_;
        std::uniform_real_distribution<> Distribution_;
    };

    // static Random random(std::random_device{}());
    static Random RANDOM(228228227);
    // static Random RANDOM_FOR_PSO(228228227, -0.1, 0.1);
}

int32_t WeightedRandomSelection(const std::vector<double>& probabilities) {
    return RANDOM.WeightedRandomSelection(probabilities);
}

double RandomUniform() {
    return RANDOM.RandomUniform();
}

int64_t RandomUniform(int64_t min, int64_t max) {
    if (max <= min) {
        return min;
    }
    auto rnd = RANDOM.RandomUniform();
    return std::round(min + rnd  * (max - min));
}

std::mt19937& RandomGenerator() {
    return RANDOM.Generator();
}


std::vector<int32_t> RandomPermutation(int64_t n) {
    std::vector<int32_t> permutation(n);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), RANDOM.Generator());
    return permutation;
}
