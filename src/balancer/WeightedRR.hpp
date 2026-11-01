#pragma once
#include "IAlgorithm.hpp"
#include <mutex>

class WeightedRR : public IAlgorithm {
    size_t current_index{0};
    int current_usage{0};
    mutex mtx;
    public:
    Backend* pick_backend(BackendPool& pool,const string& client_ip) override;
    string get_name() override;
};