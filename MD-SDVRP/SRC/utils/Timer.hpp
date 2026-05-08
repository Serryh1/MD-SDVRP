#pragma once
#include <chrono>

class GlobalTimer {
private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

    static double limit_seconds;

public:

    static void start(double limit) {
        start_time = std::chrono::high_resolution_clock::now();
        limit_seconds = limit;
    }

    static bool isTimeOut() {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current_time - start_time;
        return elapsed.count() >= limit_seconds;
    }

    static double getElapsedTime() {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current_time - start_time;
        return elapsed.count();
    }
};