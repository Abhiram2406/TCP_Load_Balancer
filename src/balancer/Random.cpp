#include "Random.hpp"
#include <stdlib.h>

Backend* Random::pick_backend(BackendPool& pool,const string& client_ip) {
    vector<size_t>store;
    for(size_t i=0;i<pool.pool.size();i++) {
        if(pool.pool[i].state==HealthState::HEALTHY) {
            store.push_back(i);
        }
    }
    if(store.empty()) {
        return nullptr;
    }
    size_t random_idx=rand()%store.size();

    return &pool.pool[store[random_idx]];
}