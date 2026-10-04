#include "HealthStateMachine.hpp"

HealthStateMachine::HealthStateMachine(int sThreshold,int fThreshold):success_threshold(sThreshold),failure_threshold(fThreshold),success_count(0),failure_count(0),state(HealthState::HEALTHY) {};

void HealthStateMachine::on_success() {
    if(state==HealthState::HEALTHY) {
        success_count++;
    }else if(state==HealthState::SUSPECTED) {
        if(success_count+1>=success_threshold) {
            state=HealthState::HEALTHY;
        }
        success_count++;
    }else{
        state=HealthState::SUSPECTED;
        success_count++;
    }
    failure_count=0;
}

void HealthStateMachine::on_failure() {
    if(state==HealthState::HEALTHY) {
        state=HealthState::SUSPECTED;
        failure_count++;
    }else if(state==HealthState::SUSPECTED) {
        if(failure_count+1>=failure_threshold) {
            state=HealthState::UNHEALTHY;
        }
        failure_count++;
    }else{
        failure_count++;
    }
    success_count=0;
}

HealthState HealthStateMachine::get_state() const {
    return state;
}