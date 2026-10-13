#pragma once
#include <iostream>
#include <vector>
#include <mutex>
#include <cmath>
using namespace std;
class Histogram {
    vector<int>counts;
    mutex count_mutex;
    vector<pair<int,int>>bins;
    public:
    Histogram();
    void record_latency(int latency);
    int count_records();
    vector<double> estimate_percentiles();
};