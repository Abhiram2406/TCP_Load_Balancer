#include "Random.hpp"
#include <stdlib.h>

Backend* Random::pick_backend(BackendPool& pool,const string& client_ip) {
    cout<<"Random algorithm called"<<endl;
    vector<size_t>store;
    for(size_t i=0;i<pool.pool.size();i++) {
        if(pool.pool[i].health_machine.get_state()==HealthState::HEALTHY) {
            store.push_back(i);
        }
    }
    if(store.empty()) {
        return nullptr;
    }
    size_t random_idx=rand()%store.size();

    return &pool.pool[store[random_idx]];
}
string Random::get_name() {
    return "Random";
}