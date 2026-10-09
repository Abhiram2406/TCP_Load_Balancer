#include "BackendPool.hpp"

void BackendPool::add_backend(const Backend& backend) {
    lock_guard<mutex>lock(pool_mutex);
    pool.emplace_back(backend.host,backend.port,backend.weight);
}
void BackendPool::remove_backend(const string& ip,int port_no) {
    lock_guard<mutex>lock(pool_mutex);
    for(int i=0;i<pool.size();i++) {
        if(pool[i].host==ip && pool[i].port==port_no) {
            pool[i].state=HealthState::UNHEALTHY;
            return;
        }
    }
}
void BackendPool::list_backend() {
    lock_guard<mutex>lock(pool_mutex);
    cout<<"----------------------------------------------------------------------"<<endl;
    cout<<"HOST_NAME"<<" | "<<"PORT"<<" | "<<"STATE"<<" | "<<"ACTIVE CONNECTIONS"<<endl;
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
        cout<<backend.host<<" | "<<backend.port<<" | "<<situation<<" | "<<backend.active_connections<<endl;
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