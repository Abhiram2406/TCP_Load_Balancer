#pragma once
#include "IAlgorithm.hpp"

class Random : public IAlgorithm {
    public:
    Backend* pick_backend(BackendPool& pool,const string& client_ip) override;
};