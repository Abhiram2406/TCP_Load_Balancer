#include "Backend.hpp"

void Backend::drain(bool sure) {
    if(sure) {
        health_machine.state=HealthState::DRAINING;
        drain_start_time=chrono::steady_clock::now();
    }else{
        health_machine.state=HealthState::SUSPECTED;
    }
}