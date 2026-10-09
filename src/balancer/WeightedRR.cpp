#include "WeightedRR.hpp"

Backend* WeightedRR::pick_backend(BackendPool& pool,const string& client_ip) {
    if (pool.pool.empty()) {
        return nullptr;
    }

    const int wt=pool.pool[current_index].weight;
    if(current_usage==wt) {
        current_index++;
        current_index=current_index%pool.pool.size();
        current_usage=0;
    }
    size_t start=current_index;
    for(size_t i=0;i<pool.pool.size();i++) {
        size_t idx=(start+i)%pool.pool.size();
        if(pool.pool[idx].health_machine.get_state()==HealthState::HEALTHY) {
            current_index+=i;
            if(i!=0) current_usage=1;
            else current_usage++;
            return &pool.pool[idx];
        }
    }

    return nullptr;
}