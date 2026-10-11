#pragma once
#include "../pool/BackendPool.hpp"
#include<thread>
class DrainMonitor {
    BackendPool &pool;
    thread checker_thread;
    int drainout_time;
    atomic<bool>running{false};
    public:
    DrainMonitor(BackendPool &pool,int drainout_time);
    void check_backend(Backend& backend);
    void start();
    void stop(); 
    void update_drainout_time(int drainout_time);
};