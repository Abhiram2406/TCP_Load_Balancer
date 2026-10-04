#pragma once
#include "HealthState.hpp"
class HealthStateMachine {
    HealthState state;
    int success_count;
    int failure_count;
    int success_threshold;
    int failure_threshold;
    public:
    HealthStateMachine(int success_threshold,int failure_threshold);
    void on_success();
    void on_failure();
    HealthState get_state() const;
};