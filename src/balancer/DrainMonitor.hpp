#pragma once
#include "../pool/BackendPool.hpp"
#include<thread>
class DrainMonitor {
    BackendPool &pool;
    thread checker_thread;
    atomic<bool>running{false};
    public:
    DrainMonitor(BackendPool &pool);
    void check_backend(Backend& backend,int drainout_time);
    void start(int drainout_time);
    void stop(); 
};