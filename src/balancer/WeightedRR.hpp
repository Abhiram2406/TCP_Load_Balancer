#pragma once
#include "IAlgorithm.hpp"
#include <atomic>

class WeightedRR : public IAlgorithm {
    atomic<size_t>current_index{0};
    atomic<int>current_usage{0};
    public:
    Backend* pick_backend(BackendPool& pool) override;
};