#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../structure/DSDVRP_function.hpp"


inline Info info;

inline void clear_info() {
    info.customer_num = 0;
    info.depots_num = 0;
    info.vehicle_num = 0;
    info.vehicle_capacity = 0;
    info.tot_demand = 0;
    info.demands.clear();
    info.customers_loc.clear();
    info.depots_loc.clear();
    info.vehicles.clear();
    info.all_loc.clear();
}

inline void read_P_set_data(const std::string& filename) {
    clear_info();

    std::ifstream infile(filename);
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string s;
    std::vector<std::vector<std::string>> res;

    while (getline(infile, s)) {
        if (s.empty()) continue;

        std::stringstream input(s);
        std::string result;
        std::vector<std::string> in;
        while (input >> result) {
            in.push_back(result);
        }
        if (!in.empty()) {
            res.push_back(in);
        }
    }
    infile.close();

    if (res.empty()) throw std::runtime_error("File is empty");

    try {
        int vehicle_per_depot = stoi(res[0][1]);
        info.customer_num = stoi(res[0][2]);
        info.depots_num = stoi(res[0][3]);

        info.vehicle_num = info.depots_num * vehicle_per_depot;
        for (int i = 0; i < info.depots_num; ++i) {
            info.vehicles.push_back(vehicle_per_depot);
        }

        if (res.size() > 1 && res[1].size() > 1) {
            info.vehicle_capacity = stoi(res[1][1]);
        } else {
            throw std::runtime_error("File format error: missing depot parameters");
        }

        int begin_line = info.depots_num + 1;

        for (int i = begin_line; i < info.customer_num + begin_line; i++) {
            if (i >= res.size()) throw std::runtime_error("Unexpected end of file reading customers");

            double x = stod(res[i][1]);
            double y = stod(res[i][2]);
            info.customers_loc.push_back(Position{x, y});
            info.demands.push_back(stoi(res[i][4]));
            info.tot_demand += info.demands.back();
        }

        int len = info.depots_num + info.customer_num;
        for (int i = len + 1; i < len + 1 + info.depots_num; i++) {
            if (i >= res.size()) throw std::runtime_error("Unexpected end of file reading depots");

            double x = stod(res[i][1]);
            double y = stod(res[i][2]);
            info.depots_loc.push_back(Position{x, y});
        }

        for (const auto& item : info.depots_loc) info.all_loc.push_back(item);
        for (const auto& item : info.customers_loc) info.all_loc.push_back(item);

    } catch (const std::exception& e) {
        throw std::runtime_error("Data parsing error: " + std::string(e.what()));
    }
}

inline void read_SD_set_data(const std::string& filename) {
    clear_info();

    std::ifstream infile(filename);
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string s;
    std::vector<std::vector<std::string>> res;
    while (getline(infile, s)) {
        if (s.empty()) continue;
        std::stringstream input(s);
        std::string result;
        std::vector<std::string> in;
        while (input >> result) {
            in.push_back(result);
        }
        if (!in.empty()) res.push_back(in);
    }
    infile.close();

    if (res.empty()) throw std::runtime_error("File is empty");

    try {
        info.customer_num = stoi(res[0][0]);
        info.vehicle_capacity = stoi(res[0][1]);

        if (res[0].size() < 2) throw std::runtime_error("Header line format error");

        info.depots_num = (int)res[0].size() - 2;
        info.vehicle_num = 0;

        for (int i = 2; i < res[0].size(); i++) {
            info.vehicles.push_back(stoi(res[0][i]));
            info.vehicle_num += info.vehicles.back();
        }

        if (res.size() < 2) throw std::runtime_error("Missing demand line");
        for (int i = 0; i < res[1].size(); i++) {
            info.demands.push_back(stoi(res[1][i]));
            info.tot_demand += info.demands.back();
        }

        int begin_line = 2;
        if (res.size() < begin_line + info.vehicles.size() + info.customer_num) {
            throw std::runtime_error("File too short for defined depots/customers");
        }
        for (int i = begin_line; i < begin_line + info.depots_num; i++) {  
            double x = stod(res[i][0]);
            double y = stod(res[i][1]);
            info.depots_loc.push_back({x, y});
        }

        begin_line = begin_line + info.depots_num;

        for (int i = begin_line; i < begin_line + info.customer_num; i++) {
            double x = stod(res[i][0]);
            double y = stod(res[i][1]);
            info.customers_loc.push_back({x, y});
        }

        for (const auto& item : info.depots_loc) info.all_loc.push_back(item);
        for (const auto& item : info.customers_loc) info.all_loc.push_back(item);

    } catch (const std::exception& e) {
        throw std::runtime_error("Data parsing error: " + std::string(e.what()));
    }
}

inline void read_Generate_data(const std::string& filename) {
    clear_info();

    std::ifstream infile(filename);
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string s;
    std::vector<std::vector<std::string>> res;
    while (getline(infile, s)) {
        if (s.empty()) continue;
        std::stringstream input(s);
        std::string result;
        std::vector<std::string> in;
        while (input >> result) {
            in.push_back(result);
        }
        if (!in.empty()) res.push_back(in);
    }
    infile.close();

    if (res.empty()) throw std::runtime_error("File is empty");

    try {
        info.customer_num = stoi(res[0][0]);
        info.depots_num = stoi(res[0][1]);
        info.vehicle_capacity = stoi(res[0][2]);

        for (int i = 1; i <= info.depots_num; ++i) {
            double x = stod(res[i][0]);
            double y = stod(res[i][1]);
            int veh = stoi(res[i][2]);
            info.all_loc.push_back({x, y});
            info.depots_loc.push_back({x, y});
            info.vehicles.push_back(veh);
            info.vehicle_num += veh;
        }
        for (int i = info.depots_num + 1; i <= info.depots_num + info.customer_num; ++i) {
            double x = stod(res[i][0]);
            double y = stod(res[i][1]);
            int demand = stoi(res[i][2]);
            info.all_loc.push_back({x, y});
            info.customers_loc.push_back({x, y});
            info.demands.push_back(demand);
            info.tot_demand += demand;
        }

    } catch (const std::exception& e) {
        throw std::runtime_error("Data parsing error: " + std::string(e.what()));
    }
}