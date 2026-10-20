#pragma once
#include "IAlgorithm.hpp"

class IpHash: public IAlgorithm {
    public:
    Backend* pick_backend(BackendPool& pool,const string& client_ip)override;
    string get_name() override;
};