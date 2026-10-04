#include <cassert>
#include "../src/health/HealthStateMachine.hpp"

int main()
{
    HealthStateMachine machine2(3, 3);

    machine2.on_failure();
    assert(machine2.get_state() == HealthState::SUSPECTED);

    machine2.on_failure();
    assert(machine2.get_state() == HealthState::SUSPECTED);

    machine2.on_success();
    assert(machine2.get_state() == HealthState::SUSPECTED);

    machine2.on_success();
    assert(machine2.get_state() == HealthState::SUSPECTED);

    machine2.on_success();
    assert(machine2.get_state() == HealthState::HEALTHY);

    return 0;
}