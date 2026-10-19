#pragma once
#include<thread>
#include<atomic>
#include "../common.hpp"
#include "../pool/BackendPool.hpp"
#include "../balancer/IAlgorithm.hpp"
#include "../metrics/MetricsRegistry.hpp"
#include "../balancer/DrainMonitor.hpp"
#include "../balancer/RoundRobin.hpp"
#include "../balancer/WeightedRR.hpp"
#include "../balancer/Random.hpp"
#include "../balancer/LeastConnections.hpp"
#include "../balancer/IpHash.hpp"
using namespace std;
class ControlServer {
    string socket_path;
    thread server_thread;
    BackendPool& pool;
    string toml_file;
    shared_ptr<IAlgorithm>algorithm;
    mutex& algorithm_mutex;
    DrainMonitor& monitor;
    MetricsRegistry& registry;
    atomic<bool>running{false};
    int server_fd=-1;
    public:
    ControlServer(string path,BackendPool& pool,string toml_file,shared_ptr<IAlgorithm>algorithm,mutex& algorithm_mutex,DrainMonitor& monitor,MetricsRegistry& registry);
    ~ControlServer();
    void start();
    void stop();
    void handle_client(int client_fd);
    string process_command(string command);
};