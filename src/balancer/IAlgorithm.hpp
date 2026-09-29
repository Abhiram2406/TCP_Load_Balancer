#pragma once
#include "../pool/BackendPool.hpp"

class IAlgorithm {
    public:
    virtual Backend* pick_backend(BackendPool& pool)=0;
    virtual ~IAlgorithm()=default;
};