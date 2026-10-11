#include "DrainMonitor.hpp"
#include<chrono>
DrainMonitor::DrainMonitor(BackendPool& p,int d_time) : pool(p),drainout_time(d_time) {};
void DrainMonitor::check_backend(Backend& backend) {
    if(backend.drain_start_time.has_value()) {
        auto elapsed = chrono::duration_cast<std::chrono::seconds>( chrono::steady_clock::now() - *backend.drain_start_time).count();
        if(elapsed>=drainout_time) {
            backend.registry.shutdown_all_connections();
        }
    }
}
void DrainMonitor::start() {
    running=true;
    thread checker_thread([this]() {
        while(running) {
            {
            lock_guard<mutex>lock(pool.pool_mutex);
            auto it=pool.pool.begin();
            while(it!=pool.pool.end()) {
                if(it->health_machine.get_state()==HealthState::DRAINING) {
                    check_backend(*(it));
                    if(it->remove_after_drain && it->active_connections.load() == 0) {
                        it=pool.pool.erase(it);
                        continue;
                    }
                }
                ++it;
            }
            }
            this_thread::sleep_for(chrono::seconds(10));
        }

    });
    checker_thread.detach();
}
void DrainMonitor::stop() {
    running=false;
}
void DrainMonitor::update_drainout_time(int d_time) {
    drainout_time=d_time;
}