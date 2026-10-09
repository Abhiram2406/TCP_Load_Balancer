#pragma once
#include "HealthState.hpp"
class RoundRobin;
class WeightedRR;
class Random;
class LeastConnections;
class IpHash;
class HealthChecker;
class BackendPool;
class DrainMonitor;
struct Backend;

class HealthStateMachine {
    // friend RoundRobin;
    // friend WeightedRR;
    // friend Random;
    // friend LeastConnections;
    // friend IpHash;
    // friend HealthChecker;
    friend Backend;
    // friend BackendPool;
    // friend DrainMonitor;
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