#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <algorithm> 

using namespace std;

struct Point {
    double x, y;
};

struct Client {
    int id;
    Point coord;
    int demand;
};

struct Depot {
    int id;
    Point coord;
    int num_vehicles;
};

struct InstanceConfig {
    string filename;
    int num_clients;
    int num_depots;
    int capacity;
};

double getRandomDouble(double min, double max) {
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}

int getRandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

void generateAndSave(const InstanceConfig& config) {
    int veh_min = 4;
    int veh_max = 100;

    int demand_raw_min = 1;
    int demand_raw_max = config.capacity + 100;

    vector<Client> clients;
    vector<Depot> depots;

    long long total_capacity = 0;

    for (int i = 0; i < config.num_depots; i++) {
        Depot d;
        d.id = i;
        d.coord.x = getRandomDouble(-1000.0, 1000.0);
        d.coord.y = getRandomDouble(-1000.0, 1000.0);
        d.num_vehicles = getRandomInt(veh_min, veh_max);
        depots.push_back(d);

        total_capacity += (long long)d.num_vehicles * config.capacity;
    }

    long long raw_total_demand = 0;
    for (int i = 0; i < config.num_clients; i++) {
        Client c;
        c.id = i;
        c.coord.x = getRandomDouble(-1000.0, 1000.0);
        c.coord.y = getRandomDouble(-1000.0, 1000.0);
        c.demand = getRandomInt(demand_raw_min, demand_raw_max);

        clients.push_back(c);
        raw_total_demand += c.demand;
    }


    double scaling_ratio = 1.0;

    if (raw_total_demand > total_capacity) {
        scaling_ratio = ((double)total_capacity / raw_total_demand) * 0.95;
    }

    long long final_total_demand = 0;

    if (scaling_ratio < 1.0) {
        for (auto& c : clients) {
            int new_demand = (int)(c.demand * scaling_ratio);
            c.demand = std::max(1, new_demand);
            final_total_demand += c.demand;
        }
    } else {
        final_total_demand = raw_total_demand;
    }

    if (final_total_demand > total_capacity) {
        long long diff = final_total_demand - total_capacity;
        for (auto& c : clients) {
            if (diff <= 0) break;
            if (c.demand > 1) {
                int reduce = std::min((long long)c.demand - 1, diff);
                c.demand -= reduce;
                diff -= reduce;
            }
        }
        final_total_demand = 0;
        for(auto& c : clients) final_total_demand += c.demand;
    }

    ofstream outFile(config.filename);
    if (!outFile.is_open()) {
        cerr << "file create failed: " << config.filename << endl;
        return;
    }

    outFile << fixed << setprecision(2);
    // 1. Header
    outFile << config.num_clients << " " << config.num_depots << " " << config.capacity << endl;

    // 2. Depots (x, y, vehicles)
    for (const auto& d : depots) {
        outFile << d.coord.x << " " << d.coord.y << " " << d.num_vehicles << endl;
    }

    // 3. Clients (x, y, scaled_demand)
    for (const auto& c : clients) {
        outFile << c.coord.x << " " << c.coord.y << " " << c.demand << endl;
    }

    outFile.close();

    cout << "file: " << config.filename << " finish. ";
    if (scaling_ratio < 1.0) {
        cout << "scale: " << scaling_ratio << ", ";
    } else {
        cout << "[no scale] ";
    }
    cout << "total capacity: " << total_capacity << " >= total demand: " << final_total_demand << endl;
}

int main() {
    srand(2333);

    struct SizePair { int c; int d; };
    vector<SizePair> sizes = {
        {500, 10},
        {750, 12},
        {1000, 14},
        {1250, 16},
        {1500, 18}
    };

    vector<int> capacities = {100, 200, 300, 400};

    vector<InstanceConfig> tasks;

    for (int i = 0; i < sizes.size(); ++i) {
        for (int j = 0; j < capacities.size(); ++j) {
            InstanceConfig cfg;
            stringstream ss;
            ss << "../input/g" << (i + 1) << "_" << (j + 1) << ".txt";

            cfg.filename = ss.str();
            cfg.num_clients = sizes[i].c;
            cfg.num_depots = sizes[i].d;
            cfg.capacity = capacities[j];

            tasks.push_back(cfg);
        }
    }

    cout << "start 20 data generation..." << endl;
    cout << "---------------------------------------------------------" << endl;

    for (const auto& task : tasks) {
        generateAndSave(task);
    }

    cout << "---------------------------------------------------------" << endl;
    cout << "all data generation finished." << endl;

    return 0;
}