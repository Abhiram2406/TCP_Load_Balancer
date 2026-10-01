#pragma once
#include "../pool/BackendPool.hpp"

class IAlgorithm {
    public:
    virtual Backend* pick_backend(BackendPool& pool,const string& client_ip)=0;
    virtual ~IAlgorithm()=default;
};