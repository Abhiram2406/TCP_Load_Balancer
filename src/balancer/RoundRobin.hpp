#pragma once
#include "IAlgorithm.hpp"
#include <atomic>

class RoundRobin : public IAlgorithm {
    atomic<size_t>current_index{1};
    public:
    Backend* pick_backend(BackendPool& pool,const string& client_ip) override;
    string get_name() override;
};