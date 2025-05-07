#pragma once

#include <cstdint>
#include <vector>
#include <random>

int32_t WeightedRandomSelection(const std::vector<double>& probabilities);

double RandomUniform();

int64_t RandomUniform(int64_t min, int64_t max);

std::mt19937& RandomGenerator();

std::vector<int32_t> RandomPermutation(int64_t n);
