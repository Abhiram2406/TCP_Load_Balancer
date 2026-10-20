#include "IpHash.hpp"

Backend* IpHash::pick_backend(BackendPool& pool, const string& client_ip) {
    vector<size_t>store;
    for(size_t i=0;i<pool.pool.size();i++) {
        if(pool.pool[i].health_machine.get_state()==HealthState::HEALTHY) {
            store.push_back(i);
        }
    }
    if(store.empty()) {
        return nullptr;
    }
    hash<string>hasher;
    size_t hash_value=hasher(client_ip);
    size_t hash_idx=hash_value%store.size();

    return &pool.pool[store[hash_idx]];
}
string IpHash::get_name() {
    return "Ip Hash";
}