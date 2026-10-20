#include "LeastConnections.hpp"
#include <bits/stdc++.h>

Backend* LeastConnections::pick_backend(BackendPool& pool,const string& client_ip) {
    vector<size_t>store;
    for(size_t i=0;i<pool.pool.size();i++) {
        if(pool.pool[i].health_machine.get_state()==HealthState::HEALTHY) {
            store.push_back(i);
        }
    }
    sort(store.begin(),store.end(),[&pool](size_t a, size_t b) {
        if(pool.pool[a].active_connections!=pool.pool[b].active_connections) {
            return pool.pool[a].active_connections>pool.pool[b].active_connections;
        }

        return pool.pool[a].weight<pool.pool[b].weight;
    });
    if(store.empty()) {
        return nullptr;
    }

    return &pool.pool[store.back()];
}
string LeastConnections::get_name() {
    return "Least Connections";
}