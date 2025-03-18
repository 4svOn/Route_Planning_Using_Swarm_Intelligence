#pragma once

#include <cstdint>
#include <vector>

int32_t WeightedRandomSelection(const std::vector<double>& probabilities);

double RandomUniform();
