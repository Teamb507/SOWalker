#ifndef _GRAPH_TIMER_H_
#define _GRAPH_TIMER_H_

#include <limits>
#include <sys/time.h>
#include <stdlib.h>
#include <string>
#include <queue>
#include <omp.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <algorithm>
#include <set>
#include "api/types.hpp"
#include "logger/logger.hpp"

typedef std::chrono::high_resolution_clock graph_clock;

struct time_entry {
    size_t top;
    std::chrono::time_point<graph_clock> beg_time;
    double sum_value;
    double max_value;
    double min_value;
    size_t count;
    std::priority_queue<double, std::vector<double>, std::greater<double> > q;
    std::string stringval;

    time_entry() {
        stringval = "null";
        top = 0;
        count = 0;
    }

    time_entry(const std::string& key) {
        stringval = key;
        top = 100;
        count = 0;
    }

    time_entry(const std::string& key, size_t cnt) {
        stringval = key;
        top = cnt;
        count = 0;
    }

    void start_time() {
        beg_time = graph_clock::now();
    }

    void stop_time() {
        auto end_time = graph_clock::now();
        double elapsed_time_ms = std::chrono::duration<double, std::milli>(end_time - beg_time).count();
        add(elapsed_time_ms);
    }

    void add(double elapsed_time_ms) {
        if(count == 0) {
            max_value = elapsed_time_ms;
            min_value = elapsed_time_ms;
            sum_value = elapsed_time_ms;
        } else {
            max_value = std::max(max_value, elapsed_time_ms);
            min_value = std::min(min_value, elapsed_time_ms);
            sum_value += elapsed_time_ms;
        }
        count++;
        q.push(elapsed_time_ms);
        if(q.size() > top) q.pop();
    }
};

/**
 * if you this walk_timer to measure time, you must register entry first */
class walk_timer {
private:
    std::vector<std::map<std::string, time_entry>> metrics;
    std::set<std::string> entrics;
public:
    walk_timer() {
        tid_t nthreads = omp_get_max_threads();
        metrics = std::vector<std::map<std::string, time_entry>>(nthreads);
    }

    void register_entry(const std::string& stringval) {
        if(entrics.find(stringval) == entrics.end()) {
            entrics.insert(stringval);
            for(auto & metric : metrics) metric[stringval] = time_entry(stringval);
        }
    }

    void start_time(const std::string& stringval) {
        tid_t tid = omp_get_thread_num();
        metrics[tid][stringval].start_time();
    }

    void stop_time(const std::string& stringval) {
        tid_t tid = omp_get_thread_num();
        metrics[tid][stringval].stop_time();
    }

    void report() {
        std::cout << "=================================================="<< std::endl;
        std::cout << "metrics report" << std::endl;
        for(const auto & entry : entrics) {
            double max_value = std::numeric_limits<double>::min(), min_value = std::numeric_limits<double>::max(), sum_value = 0.0;
            size_t cnt = 0, total_count = 0;
            for(auto& metric : metrics) {
                max_value = std::max(max_value, metric[entry].max_value);
                min_value = std::min(min_value, metric[entry].min_value);
                total_count += metric[entry].count;
                auto q = metric[entry].q;
                while(!q.empty()) {
                    sum_value += q.top();
                    q.pop();
                    cnt++;
                }
            }
            std::cout << entry << " time elapased count : " << total_count << ", max value : " << max_value << ", min value : " << min_value << ", average time : " << sum_value / cnt << std::endl;
        }
        std::cout << "=================================================="<< std::endl;
    }
};

class graph_timer {
private:
    timeval start, end;
public:
    graph_timer() {    }

    void start_time() {
        gettimeofday(&start, NULL);
    }

    void stop_time() {
        gettimeofday(&end, NULL);
    }

    double runtime() {
        this->stop_time();
        return end.tv_sec - start.tv_sec + ((double)(end.tv_usec - start.tv_usec)) / 1.0e6;
    }
};
#endif
