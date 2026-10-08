#pragma once
#include "HealthState.hpp"
class RoundRobin;
class WeightedRR;
class Random;
class LeastConnections;
class IpHash;
class HealthChecker;

class HealthStateMachine {
    friend RoundRobin;
    friend WeightedRR;
    friend Random;
    friend LeastConnections;
    friend IpHash;
    friend HealthChecker;
    private:
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