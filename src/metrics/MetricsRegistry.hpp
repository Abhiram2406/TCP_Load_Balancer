#pragma once
#include<atomic>
using namespace std;
class MetricsRegistry {
    atomic<uint64_t>total_connections{0};
    atomic<int>active_connections{0};
    atomic<uint64_t>total_errors{0};
    public:
    void record_connection();
    void update_connection(bool done);
    void record_error();
    uint64_t get_total_connections();
    int get_active_connections();
    uint64_t get_total_errors();
};