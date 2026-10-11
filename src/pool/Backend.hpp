#pragma once
#include <iostream>
#include <atomic>
#include<optional>
#include "../health/HealthStateMachine.hpp"
#include"../net/ConnectionRegistry.hpp"
using namespace std;

struct Backend {
    string host;
    int port;
    int weight;
    HealthState state;
    atomic<int>active_connections{0};
    HealthStateMachine health_machine;
    ConnectionRegistry registry;
    optional<chrono::steady_clock::time_point>drain_start_time;
    bool remove_after_drain=false;
    Backend(string ip,int port_no):host(ip),port(port_no),weight(1),state(HealthState::HEALTHY),health_machine(3,3) {};
    Backend(string ip,int port_no,int wt):host(ip),port(port_no),weight(wt),state(HealthState::HEALTHY),health_machine(3,3) {};
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
    void drain();
};