#pragma once
#include <iostream>
#include <vector>
using namespace std;

struct BackendConfig {
    string host;
    int port;
    int weight;
};

struct Config {
    int listen_port;
    string algorithm;
    vector<BackendConfig>backends;
};