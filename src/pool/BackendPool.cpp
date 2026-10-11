#include "BackendPool.hpp"
#include<map>

void BackendPool::add_backend(const Backend& backend) {
    lock_guard<mutex>lock(pool_mutex);
    pool.emplace_back(backend.host,backend.port,backend.weight);
}
void BackendPool::remove_backend(const string& ip,int port_no) {
    lock_guard<mutex>lock(pool_mutex);
    for(int i=0;i<pool.size();i++) {
        if(pool[i].host==ip && pool[i].port==port_no) {
            pool[i].drain();
            pool[i].remove_after_drain=true;
            return;
        }
    }
}
void BackendPool::list_backend() {
    lock_guard<mutex>lock(pool_mutex);
    cout<<"----------------------------------------------------------------------"<<endl;
    cout<<"HOST_NAME"<<" | "<<"PORT"<<" | "<<"STATE    "<<" | "<<"ACTIVE CONNECTIONS"<<" | "<<"WEIGHT"<<endl;
    cout<<"----------------------------------------------------------------------"<<endl;
    for(const auto &backend : pool) {
        string situation;
        switch (backend.health_machine.get_state()) {
            case HealthState::HEALTHY:
                situation = "HEALTHY  ";
                break;

            case HealthState::DRAINING:
                situation = "DRAINING ";
                break;

            case HealthState::UNHEALTHY:
                situation = "UNHEALTHY";
                break;
            case HealthState::SUSPECTED:
                situation = "SUSPECTED";
                break;
        }
        cout<<backend.host<<" | "<<backend.port<<" | "<<situation<<" | "<<backend.active_connections<<"                  | "<<backend.weight<<endl;
    }
    cout<<"----------------------------------------------------------------------"<<endl;
}
void BackendPool::drain_backend(const string& h,int p) {
    lock_guard<mutex>lock(pool_mutex);
    for(auto &backend: pool) {
        if(backend.host==h && backend.port==p) {
            backend.drain();
            cout<<"BACKEND DRAINED SUCCESSFULLY."<<endl;
            return;
        }
    }
    cout<<"NO BACKEND FOUND WITH THE PROVIDED HOST AND PORT"<<endl;
}
void BackendPool::update_backends(const vector<BackendConfig>&backends) {
    lock_guard<mutex>lock(pool_mutex);
    vector<bool>check(pool.size(),false);
    map<pair<string,int>,size_t>mp;
    for(size_t i=0;i<pool.size();i++) {
        mp[{pool[i].host,pool[i].port}]=i;
    }
    for(auto &backend:backends) {
        if(mp.find({backend.host,backend.port})!=mp.end()) {
            check[mp[{backend.host,backend.port}]]=true;
            pool[mp[{backend.host,backend.port}]].weight=backend.weight;
        }else{
            pool.emplace_back(backend.host,backend.port,backend.weight);
            check.push_back(true);
        }
    }
    for(size_t i=0;i<check.size();i++) {
        if(!check[i]) {
            pool[i].drain();
            pool[i].remove_after_drain=true;
        }
    }
}