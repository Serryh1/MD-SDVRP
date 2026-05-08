#pragma once

#include <vector>



#define DEPOT_ENUMS_LIMIT 1000000
struct Position {
    double x;
    double y;
};

struct Info {
    int customer_num;                     // customer number
    int vehicle_capacity;                         // vehicle capacity, in this problem it is the same for all vehicles
    int depots_num;                       // depot number
    int vehicle_num;    
    int tot_demand;                  // vehicle number
    std::vector<int> vehicles;            // vehicle number in each depot
    std::vector<int> demands;             // demand of each customer
    std::vector<Position> all_loc;        // locations of depots and customers
    std::vector<Position> depots_loc;     // locations of depots
    std::vector<Position> customers_loc;  // locations of customers
};
// init the nodes for customers and depots
struct CITIES {
    double x, y;
    int demand;
};
struct TREE {
    std::vector<std::vector<int>> adj;
    double totcost;
};

struct Edge {
    int u;
    int v;
    double cost;
};

struct Component {
    int root;                // root is the depot number;
    std::vector<int> nodes;  // the customers and depot in this component
};

// To store the edges for each components  to components
struct TreeEdgeConnect {
    int com_u;
    int u;
    int com_v;
    int v;
    double cost;
};

struct DemandAssignmentToCustomer {
    int customer;
    int assignmentdemand;
};

struct Cycles {
    std::vector<DemandAssignmentToCustomer> path;
    int cnt_path;
    // if the size of path is equal to 1
    // we define them as only one node
};

struct Flow{
    int u;
    int v;
    int flow;
};