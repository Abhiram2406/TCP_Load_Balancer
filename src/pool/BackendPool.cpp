#pragma once
#include "BackendPool.hpp"

void BackendPool::add_backend(const Backend& backend) {
    lock_guard<mutex>lock(pool_mutex);
    pool.push_back(backend);
}
void BackendPool::remove_backend(const string& ip,int port_no) {
    lock_guard<mutex>lock(pool_mutex);
    for(int i=0;i<pool.size();i++) {
        if(pool[i].host==ip && pool[i].port==port_no) {
            pool[i].state=BackendState::DRAINING;
            return;
        }
    }
}
void BackendPool::list_backend() {
    lock_guard<mutex>lock(pool_mutex);
    cout<<"HOST_NAME"<<" | "<<"PORT"<<" | "<<"STATE"<<" | "<<"ACTIVE CONNECTIONS"<<endl;
    cout<<"----------------------------------------------------------------------"<<endl;
    for(const auto &backend : pool) {
        string situation;
        switch (backend.state) {
            case BackendState::HEALTHY:
                situation = "HEALTHY";
                break;

            case BackendState::DRAINING:
                situation = "DRAINING";
                break;

            case BackendState::UNHEALTHY:
                situation = "UNHEALTHY";
                break;
        }
        cout<<backend.host<<" | "<<backend.port<<" | "<<situation<<" | "<<backend.active_connections<<endl;
    }
}