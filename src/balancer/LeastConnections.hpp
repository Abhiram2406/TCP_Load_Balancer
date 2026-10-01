#pragma once
#include "IAlgorithm.hpp"

class LeastConnections : public IAlgorithm {
    public:
    Backend* pick_backend(BackendPool& pool) override;
};