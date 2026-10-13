#include "Histogram.hpp"
int Histogram::count_records() {
    int total_records=0;
    lock_guard<mutex>lock(count_mutex);
    for(int count:counts) {
        total_records+=count;
    }

    return total_records;
}
Histogram::Histogram() {
    counts.resize(7,0);
    for(int i=0;i<7;i++) {
        switch (i)
        {
        case 0:
            bins.push_back({0,5});
            break;
        
        case 1:
            bins.push_back({6,10});
            break;
        
        case 2:
            bins.push_back({11,25});
            break;
        
        case 3:
            bins.push_back({26,50});
            break;
        
        case 4:
            bins.push_back({51,100});
            break;
        
        case 5:
            bins.push_back({101,250});
            break;
        
        case 6:
            bins.push_back({251,INT32_MAX});
            break;
        
        default:
            break;
        }
    }
}
void Histogram::record_latency(int latency) {
    lock_guard<mutex>lock(count_mutex);
    for(int i=0;i<counts.size();i++) {
        if(bins[i].first<=latency && bins[i].second>=latency) {
            counts[i]++;
            break;
        }
    }
}
vector<double> Histogram::estimate_percentiles() {
    lock_guard<mutex> lock(count_mutex);

    int total = 0;
    for (int count : counts) {
        total += count;
    }
    if (total == 0) {
        return {0,0,0};
    }

    vector<double> percentiles = {50, 95, 99};
    vector<double>ans;
    for (double p : percentiles) {
        int target = ceil((p / 100.0) * total);
        int cumulative = 0;

        for (int i = 0; i < counts.size(); i++) {
            cumulative += counts[i];
            if (cumulative >= target) {
                double estimate;
                if (i == 0) {
                    estimate = (bins[i].first + bins[i].second) / 2.0;
                } else {
                    int previous = cumulative - counts[i];
                    if (i == 6) {
                        estimate = bins[i].first;
                    } else if (i == 0) {
                        estimate = (bins[i].first + bins[i].second) / 2.0;
                    } else {
                        int previous = cumulative - counts[i];
                        double fraction =(double)(target - previous) / counts[i];
                        estimate = bins[i].first +fraction * (bins[i].second - bins[i].first);
                    }
                }
                ans.push_back(estimate);
                break;
            }
        }  
    }
    return ans;
}