#pragma once
#include "Backend.hpp"
#include <vector>
#include <mutex>
using namespace std;

class RoundRobin;
class WeightedRR;
class Random;
class LeastConnections;
class IpHash;
class HealthChecker;
class DrainMonitor;

class BackendPool {
    friend RoundRobin;
    friend WeightedRR;
    friend Random;
    friend LeastConnections;
    friend IpHash;
    friend HealthChecker;
    friend DrainMonitor;
    private:
    vector<Backend>pool;
    mutex pool_mutex;
    public:
    void add_backend(const Backend& backend);
    void remove_backend(const string& host,int port);
    void list_backend();
    void drain_backend(const string& host, int port);
};