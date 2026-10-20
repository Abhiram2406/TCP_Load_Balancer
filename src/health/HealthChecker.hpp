#pragma once
#include "../pool/BackendPool.hpp"
#include <chrono>
#include <thread>

class HealthChecker {
    BackendPool &pool;
    thread checker_thread;
    atomic<bool>running{false};
    public:
    HealthChecker(BackendPool &pool);
    void check_backend(Backend& backend);
    void start();
    void stop();
    void force_check();
};