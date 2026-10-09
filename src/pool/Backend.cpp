#include "Backend.hpp"

void Backend::drain() {
    health_machine.state=HealthState::DRAINING;
    drain_start_time=chrono::steady_clock::now();
}