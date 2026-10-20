#include "HealthChecker.hpp"
#include "../common.hpp"

HealthChecker::HealthChecker(BackendPool& p):pool(p) {};
void HealthChecker::check_backend(Backend& backend) {
    int sock_fd=socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0) {
        return;
    }
    sockaddr_in server_address{};
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(backend.port);
    inet_pton(AF_INET,backend.host.c_str(),&server_address.sin_addr);

    int result=connect(sock_fd,(sockaddr*)&server_address,sizeof(server_address));

    if(result==0) {
        // cout<<"backend "<<backend.port<<"success"<<endl;
        backend.health_machine.on_success();
    }else{
        // cout<<"backend "<<backend.port<<"failure"<<endl;
        backend.health_machine.on_failure();
    }
    close(sock_fd);
}
void HealthChecker::start() {
    running=true;
    thread checker_thread([this]() {
        while(running) {
        for(auto &backend : pool.pool) {
            if(backend.health_machine.get_state()!=HealthState::DRAINING)
            check_backend(backend);
        }
        this_thread::sleep_for(chrono::seconds(10));
    }
    });
    checker_thread.detach();
}
void HealthChecker::stop() {
    running=false;
}
void HealthChecker::force_check() {
    if(running) {
        return;
    }
    thread checker_thread([this]() {
        for(auto &backend : pool.pool) {
            if(backend.health_machine.get_state()!=HealthState::DRAINING)
            check_backend(backend);
        }
    });
    checker_thread.detach();
}