#include "RoundRobin.hpp"

Backend* RoundRobin::pick_backend(BackendPool& pool_array,const string& client_ip) {
    if (pool_array.pool.empty()) return nullptr;

    size_t start=current_index;
    current_index++;
    for(size_t i=0;i<pool_array.pool.size();i++) {
        size_t idx=(start+i)%pool_array.pool.size();
        if(pool_array.pool[idx].state==HealthState::HEALTHY) {
            return &pool_array.pool[idx];
        }
    }

    return nullptr;
}