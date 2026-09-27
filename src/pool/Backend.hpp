#pragma once
#include <iostream>
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
    int active_connections;
};