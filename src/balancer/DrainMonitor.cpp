#include "DrainMonitor.hpp"
#include<chrono>
DrainMonitor::DrainMonitor(BackendPool& p) : pool(p) {};
void DrainMonitor::check_backend(Backend& backend,int drainout_time) {
    if(backend.drain_start_time.has_value()) {
        auto elapsed = chrono::duration_cast<std::chrono::seconds>( chrono::steady_clock::now() - *backend.drain_start_time).count();
        if(elapsed>=drainout_time) {
            backend.registry.shutdown_all_connections();
        }
    }
}
void DrainMonitor::start(int drainout_time) {
    running=true;
    thread checker_thread([this,drainout_time]() {
        while(running) {
        for(auto &backend : pool.pool) {
            if(backend.health_machine.get_state()==HealthState::DRAINING)
            check_backend(backend,drainout_time);
        }
        this_thread::sleep_for(chrono::seconds(10));
    }
    });
    checker_thread.detach();
}
void DrainMonitor::stop() {
    running=false;
}