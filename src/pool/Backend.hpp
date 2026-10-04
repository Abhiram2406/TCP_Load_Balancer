#pragma once
#include <iostream>
#include <atomic>
#include "../health/HealthStateMachine.hpp"
using namespace std;

enum class BackendState {
    HEALTHY,
    SUSPECTED,
    UNHEALTHY,
    DRAINING
};

struct Backend {
    string host;
    int port;
    int weight;
    BackendState state;
    atomic<int>active_connections{0};
    HealthStateMachine health_machine;
    Backend(string ip,int port_no):host(ip),port(port_no),weight(1),state(BackendState::HEALTHY),health_machine(3,3) {};
    Backend(string ip,int port_no,int wt):host(ip),port(port_no),weight(wt),state(BackendState::HEALTHY),health_machine(3,3) {};
    Backend(Backend&& other) noexcept
        : host(move(other.host)),
        port(other.port),
        weight(other.weight),
        state(other.state),
        active_connections(other.active_connections.load()),
        health_machine(std::move(other.health_machine)) {}

    Backend& operator=(Backend&& other) noexcept {
        if (this != &other) {
            host = move(other.host);
            port = other.port;
            weight = other.weight;
            state = other.state;
            active_connections.store(other.active_connections.load());
            health_machine = std::move(other.health_machine);
        }
        return *this;
    }
};