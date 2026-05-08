#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../algorithm/GW.hpp"
#include "../algorithm/MCMF.hpp"
#include "../algorithm/Partition.hpp"
#include "../blossom/PerfectMatching.h"
#include "../structure/DSDVRP_function.hpp"
#include "../utils/Timer.hpp"

inline std::vector<int> father;

inline int find_father(int x) {
    if (father[x] != x) {
        father[x] = find_father(father[x]);
    }
    return father[x];
}

// To connect commponts
inline TreeEdgeConnect getMinTreeEdge(int com_u, int com_v) {
    double min_cost = INFINITE;
    TreeEdgeConnect tree_edge_connect;

    tree_edge_connect.com_u = com_u;
    tree_edge_connect.com_v = com_v;

    const auto& nodes_u = component_from_spanning_forest[com_u].nodes;
    const auto& nodes_v = component_from_spanning_forest[com_v].nodes;
    for (const auto& u : nodes_u) {
        for (const auto& v : nodes_v) {
            if (u < depotsnum || v < depotsnum) continue;  // both are customers
            double cost = adjMatrix[u][v];
            if (cost < min_cost) {
                min_cost = cost;
                tree_edge_connect.u = u;
                tree_edge_connect.v = v;
                tree_edge_connect.cost = cost;
            }
        }
    }
    return tree_edge_connect;
}

inline bool cmpTreeEdgeConnect(const TreeEdgeConnect& a, const TreeEdgeConnect& b) { return a.cost < b.cost; }

inline void getEulerPath(int u, std::vector<std::vector<int>>& adj, std::vector<int>& walk) {
    std::stack<int> st;
    st.push(u);

    while (!st.empty()) {
        int v = st.top();
        if (!adj[v].empty()) {
            int neighbor = adj[v].back();
            adj[v].pop_back();
            for (auto it = adj[neighbor].begin(); it != adj[neighbor].end(); ++it) {
                if (*it == v) {
                    *it = adj[neighbor].back();
                    adj[neighbor].pop_back();
                    break;
                }
            }

            st.push(neighbor);
        } else {
            walk.push_back(v);
            st.pop();
        }
    }
    reverse(walk.begin(), walk.end());
}

inline std::vector<std::vector<int>> partitionToNewForest(const int& partition_i) {
    TREE new_forest = origin_forest;
    // initalize the father arrays
    int components_num = component_from_spanning_forest.size();
    for (int i = 0; i < components_num; i++) {
        father[i] = i;
    }

    std::vector<TreeEdgeConnect> tree_edge_connects;
    int tot_edges = 0;
    int partitions_size = partitions[partition_i].size();
    for (int i = 0; i < partitions_size; i++) {
        const auto& vec = partitions[partition_i][i];
        int siz = vec.size();
        tot_edges += siz - 1;
        if (siz == 1) continue;
        // we need to connect all nodes in vec
        for (int com_u_vid = 0; com_u_vid < siz; com_u_vid++) {
            int com_u = vec[com_u_vid];
            for (int com_v_vid = com_u_vid + 1; com_v_vid < siz; com_v_vid++) {
                int com_v = vec[com_v_vid];
                // find the minimum cost edge between component u and component v
                tree_edge_connects.push_back(getMinTreeEdge(com_u, com_v));
            }
        }
    }
    // kruskal to combine the trees
    std::sort(tree_edge_connects.begin(), tree_edge_connects.end(), cmpTreeEdgeConnect);
    int add_edges = 0;
    for (int i = 0; i < tree_edge_connects.size(); i++) {
        int com_u = tree_edge_connects[i].com_u;
        int com_v = tree_edge_connects[i].com_v;
        int fa_u = find_father(com_u);
        int fa_v = find_father(com_v);
        if (fa_u != fa_v) {
            father[fa_v] = fa_u;
            // add edge to new forest
            int u = tree_edge_connects[i].u;
            int v = tree_edge_connects[i].v;
            new_forest.adj[u].push_back(v);
            new_forest.adj[v].push_back(u);
            new_forest.totcost += tree_edge_connects[i].cost;
            add_edges++;
        }
    }
    // to ensure there ia still a forest
    if (add_edges != tot_edges) {
        throw std::runtime_error("Error in forming new forest from partition!");
    }

    std::vector<std::vector<int>> pathsFromForest;
    unordered_set<int> unique_nodes;
    int customer_num = 0;
    for (int i = 0; i < components_num; i++) {
        if (1) {
            std::vector<int> walk, vec;
            // dfsEuler(i, new_forest.adj, walk);
            getEulerPath(i, new_forest.adj, walk);
            if (walk.size() <= 1) continue;
            // erase the same nodes
            // cout << '\n';
            for (const auto& node : walk) {
                // std::cout << node << "->";
                if (!unique_nodes.count(node) && node >= depotsnum) {
                    vec.push_back(node);
                    unique_nodes.insert(node);
                    customer_num++;
                }
            }
            // cout << '\n';
            pathsFromForest.push_back(vec);
        }
    }
    if (customer_num != customersnum) {
        throw std::runtime_error("Error in path constructing!");
    }
    return pathsFromForest;
}

inline void printPathsFromForest(const std::vector<std::vector<int>>& paths) {
    std::cout << "Paths from new forest: " << std::endl;
    for (const auto& path : paths) {
        for (const auto& node : path) {
            std::cout << node << " ";
        }
        std::cout << std::endl;
    }
}

inline std::vector<Cycles> cycleSpiltToPaths(const std::vector<std::vector<int>>& cycle) {
    std::vector<Cycles> paths;
    int Q = info.vehicle_capacity;
    std::vector<int> copy_demands = info.demands;

    std::vector<DemandAssignmentToCustomer> path;
    int need_vehicles = 0;
    // if demand of a customer >= Q, split it first
    // one node is define as a trival path
    for (int i = 0; i < copy_demands.size(); i++) {
        if (copy_demands[i] >= Q) {
            int full_load_times = copy_demands[i] / Q;
            int remaining_demand = copy_demands[i] % Q;
            int customer_id = i + depotsnum;

            DemandAssignmentToCustomer dac;
            dac.customer = customer_id;
            dac.assignmentdemand = Q;
            path.push_back(dac);
            Cycles new_cycle;
            new_cycle.path = path;
            new_cycle.cnt_path = full_load_times;
            paths.push_back(new_cycle);

            path.clear();
            // there exist demand for customer i
            copy_demands[i] = remaining_demand;
            need_vehicles += full_load_times;
        }
    }
    Cycles tmp_cycle;
    bool is_ok = true;
    for (const auto& cyc : cycle) {
        int current_load = 0;

        // ****************************
        // check the number of vehicle is satify
        int need = 0;
        for (const auto& customer : cyc) {
            int demand = copy_demands[customer - depotsnum];
            need += demand;
        }
        need_vehicles += (need + Q - 1) / Q;

        if (need_vehicles > info.vehicle_num) {
            is_ok = false;
            break;
        }
        // ****************************

        for (const auto& customer : cyc) {
            int demand = copy_demands[customer - depotsnum];

            if (demand == 0) continue;

            if (current_load + demand <= Q) {
                DemandAssignmentToCustomer dac;
                dac.customer = customer;
                dac.assignmentdemand = demand;
                path.push_back(dac);

                current_load += demand;
                copy_demands[customer - depotsnum] = 0;
                if (current_load == Q) {
                    tmp_cycle.path = path;
                    tmp_cycle.cnt_path = 1;
                    paths.push_back(tmp_cycle);
                    path.clear();
                    current_load = 0;
                }
            } else {
                int remaining_capacity = Q - current_load;
                // fill the remaining capacity
                DemandAssignmentToCustomer dac;
                dac.customer = customer;
                dac.assignmentdemand = remaining_capacity;
                path.push_back(dac);
                tmp_cycle.path = path;
                tmp_cycle.cnt_path = 1;
                paths.push_back(tmp_cycle);
                path.clear();
                if (demand - remaining_capacity > 0) {
                    current_load = demand - remaining_capacity;
                    // update the demand of this customer
                    dac.assignmentdemand = current_load;
                    dac.customer = customer;
                    path.push_back(dac);
                } else {
                    current_load = 0;
                }
            }
        }
        if (path.size() > 0) {
            tmp_cycle.path = path;
            tmp_cycle.cnt_path = 1;
            paths.push_back(tmp_cycle);
            path.clear();
        }
    }
    std::vector<Cycles> tmp_paths;
    if (!is_ok) {
        return tmp_paths;
    }
    return paths;
}

inline void printDemandAssignmentPaths(const std::vector<Cycles>& paths) {
    // std::cout << "Demand Assignment Paths: " << std::endl;
    std::unordered_map<int, int> customer_demand_map;
    for (const auto& tmp_cycle : paths) {
        const auto& path = tmp_cycle.path;
        const int& cnt_path = tmp_cycle.cnt_path;
        for (const auto& dac : path) {
            // std::cout << "(" << dac.customer << ", " << dac.assignmentdemand << ") ";
            if (dac.customer >= depotsnum) {
                customer_demand_map[dac.customer - depotsnum] += dac.assignmentdemand * cnt_path;
            }
            if (dac.assignmentdemand < 0) {
                throw std::runtime_error("Error: Assigned demand should be positive!");
            }
        }
        // std::cout << "rounds is : " << cnt_path;
        // std::cout << std::endl;
    }

    // Verify that the total assigned demand matches the original demand for each customer
    for (const auto& [customer_idx, total_assigned_demand] : customer_demand_map) {
        int original_demand = info.demands[customer_idx];
        if (total_assigned_demand != original_demand) {
            throw std::runtime_error("Error: Total assigned demand does not match original demand for customer " +
                                     std::to_string(customer_idx));
        }
    }
}

inline std::vector<std::vector<DemandAssignmentToCustomer>> TmpRoute;
inline std::vector<std::vector<DemandAssignmentToCustomer>> MinCostRoute;
inline double getMinCostRoute(int depot_id, const std::vector<DemandAssignmentToCustomer>& vec) {
    int vec_size = vec.size();

    double tot_cost = 0;
    int nearest_customer_vid = 0;
    double nearest_distance = INFINITE;
    for (int i = 0; i < vec_size; ++i) {
        int customer = vec[i].customer;
        double dist = GetEdgeVal(info.depots_loc[depot_id], info.all_loc[customer]);
        if (nearest_distance - dist >= 1e-6) {
            nearest_distance = dist;
            nearest_customer_vid = i;
        }
    }
    std::vector<DemandAssignmentToCustomer> tmp_route_next, tmp_route_fore;
    double cost_fore = 0, cost_next = 0;
    // next
    tmp_route_next.push_back(DemandAssignmentToCustomer{.customer = depot_id, .assignmentdemand = 0});

    int cnt_nxt = vec.size();
    int idx_nxt = nearest_customer_vid;
    while (cnt_nxt--) {
        auto last_pos = info.all_loc[tmp_route_next.back().customer];
        auto now_pos = info.all_loc[vec[idx_nxt].customer];
        double dist = GetEdgeVal(last_pos, now_pos);
        cost_next += dist;
        tmp_route_next.push_back(DemandAssignmentToCustomer{
            .customer = vec[idx_nxt].customer,
            .assignmentdemand = vec[idx_nxt].assignmentdemand,
        });
        idx_nxt++;
        if (idx_nxt == vec_size) {
            idx_nxt = 0;
        }
    }
    auto last_pos = info.all_loc[tmp_route_next.back().customer];
    auto now_pos = info.all_loc[depot_id];
    cost_next += GetEdgeVal(last_pos, now_pos);
    // The vehicle need to back to the depot
    tmp_route_next.push_back(DemandAssignmentToCustomer{.customer = depot_id, .assignmentdemand = 0});

    // fore
    tmp_route_fore.push_back(DemandAssignmentToCustomer{.customer = depot_id, .assignmentdemand = 0});
    int cnt_fore = vec.size();
    int idx_fore = nearest_customer_vid;
    while (cnt_fore--) {
        auto last_pos = info.all_loc[tmp_route_fore.back().customer];
        auto now_pos = info.all_loc[vec[idx_fore].customer];

        double dist = GetEdgeVal(last_pos, now_pos);
        cost_fore += dist;
        tmp_route_fore.push_back(DemandAssignmentToCustomer{
            .customer = vec[idx_fore].customer,
            .assignmentdemand = vec[idx_fore].assignmentdemand,
        });

        idx_fore--;
        if (idx_fore < 0) {
            idx_fore = vec_size - 1;
        }
    }
    last_pos = info.all_loc[tmp_route_fore.back().customer];
    now_pos = info.all_loc[depot_id];
    cost_fore += GetEdgeVal(last_pos, now_pos);
    tmp_route_fore.push_back(DemandAssignmentToCustomer{.customer = depot_id, .assignmentdemand = 0});

    if (cost_fore - cost_next >= 1e-3) {
        tot_cost = cost_next;
        TmpRoute.push_back(tmp_route_next);
    } else {
        tot_cost = cost_fore;
        TmpRoute.push_back(tmp_route_fore);
    }
    return tot_cost;
}

inline void printRoute(const std::vector<int>& assignmentvehicles) {
    cout << "The Route is: \n";
    std::unordered_map<int, int> check_demand;
    for (int i = 0; i < TmpRoute.size(); ++i) {
        const auto& vec = TmpRoute[i];
        int rounds = assignmentvehicles[i];
        int service = 0;
        for (int j = 0; j < vec.size(); ++j) {
            const auto& [u, v] = vec[j];
            if (j == 0) {
                std::cout << "depot is : " << u << "   ";
                // std::cout << " the customer's path is: ";
            } else {
                if (u >= depotsnum) {
                    std::cout << u << "_" << v << ' ';
                    check_demand[u - depotsnum] += v * rounds;
                    service += v * rounds;
                } else
                    cout << "depot_" << u << ' ';
            }
        }
        cout << "( X " << rounds << ") ";
        cout << "the tot demand is : " << service;
        cout << '\n';
    }
    for (auto [_, demand] : check_demand) {
        if (demand != info.demands[_]) {
            throw std::runtime_error("Error: there exist a customer that don't satify demand. the customer is :  " +
                                     std::to_string(_));
        }
    }
}

inline void printMinRoute(const std::vector<int>& assignmentvehicles) {
    cout << "The Route is: \n";
    std::unordered_map<int, int> check_demand;
    for (int i = 0; i < MinCostRoute.size(); ++i) {
        const auto& vec = MinCostRoute[i];
        int rounds = assignmentvehicles[i];
        int service = 0;
        for (int j = 0; j < vec.size(); ++j) {
            const auto& [u, v] = vec[j];
            if (j == 0) {
                std::cout << "depot is : " << u << "   ";
                // cout << " the customer's path is: ";
            } else {
                if (u >= depotsnum) {
                    std::cout << u << "_" << v << ' ';
                    check_demand[u - depotsnum] += v * rounds;
                    service += v * rounds;
                }
                if (j == vec.size() - 1) {
                    cout << "depot_" << u << ' ';
                }
            }
        }
        cout << "( X " << rounds << ") ";
        cout << "the tot demand is : " << service;
        cout << '\n';
    }
    for (auto [c, demand] : check_demand) {
        if (demand != info.demands[c]) {
            throw std::runtime_error("Error: there exist a customer that don't satify demand. the customer is :  " +
                                     std::to_string(c));
        }
    }
}

inline void initRoute() {
    for (int i = 0; i < TmpRoute.size(); ++i) {
        TmpRoute.clear();
    }
}
// *************************************************************
// adding min cost edges
inline void computeResultByAddingMinCostEdges() {
    int partitions_size = partitions.size();
    int components_num = component_from_spanning_forest.size();
    father.resize(components_num);
    double min_cost = INFINITE;

    std::vector<int> assignments;

    // for each partition to find a route
    for (int i = 0; i < partitions_size; ++i) {
        auto euler_paths_from_forest = partitionToNewForest(i);
        // printPathsFromForest(euler_paths_from_forest);

        auto demand_assignment_paths = cycleSpiltToPaths(euler_paths_from_forest);

        // this scheme needs more vehicles
        if (demand_assignment_paths.size() == 0) {
            std::cout << "partition " << i << " can not be satify! \n";
            continue;
        }

        // printDemandAssignmentPaths(demand_assignment_paths);

        auto depots_to_paths = minCostMaxFlow(demand_assignment_paths);
        auto printDepotToPath = [&](int x) {
            for (const auto& flow_sol : depots_to_paths) {
                int u = flow_sol.u;
                int v_id = flow_sol.v;
                int flow = flow_sol.flow;
                int v = v_id - depotsnum;
                cout << "depot is : " << u << " to choose the demand assignment path " << v << '\n';
                cout << "The path is : " << '\n';
                for (const auto& [customer_id, demand] : demand_assignment_paths[v].path) {
                    cout << "( " << customer_id << " , " << demand << ")  ";
                }
                cout << "the cnt_path is: " << flow;
                cout << '\n';
            }
        };
        // printDepotToPath(0);

        initRoute();
        double cost = 0;
        std::vector<int> assignmentvehicles;
        for (const auto& flow_sol : depots_to_paths) {
            int depot_id = flow_sol.u;
            int v_id = flow_sol.v;
            int flow = flow_sol.flow;

            int v = v_id - depotsnum;
            auto path = demand_assignment_paths[v].path;
            cost += getMinCostRoute(depot_id, path) * flow;
            assignmentvehicles.push_back(flow);
        }
        // std::cout << "\n\n\n";
        // std::cout << "The tot cost is " << cost << '\n';
        // printRoute(assignmentvehicles);

        if (min_cost - cost > 1e-3) {
            min_cost = cost;
            MinCostRoute = TmpRoute;
            assignments = assignmentvehicles;
        }
    }

    std::cout << "\n\n\nThe minimum cost route is : " << min_cost << '\n';
    printMinRoute(assignments);
}
// *************************************************************
// using the min cost perfect matching

inline std::vector<Edge> MinimumCostPerfectMatching(const std::vector<int>& Odds) {
    int u, v;
    int sz = Odds.size();
    // Graph G(n);
    int edge_num = sz * (sz - 1) / 2;
    PerfectMatching* pm = new PerfectMatching(sz, edge_num);

    for (int i = 0; i < sz; i++) {
        for (int j = i + 1; j < sz; j++) {
            int u = Odds[i];
            int v = Odds[j];
            int c = (int)(adjMatrix[u][v] * 10);
            pm->AddEdge(i, j,c);
        }
    }
    std::vector<Edge> result;

    pm->Solve();
    std::unordered_set<int> matched;
    for (int i = 0; i < sz; ++i) {
        if (!matched.count(Odds[i])) {
            Edge edge;
            edge.u = Odds[i];
            edge.v = Odds[pm->GetMatch(i)];
            edge.cost = adjMatrix[edge.u][edge.v];
            result.push_back(edge);
            matched.insert(edge.u);
            matched.insert(edge.v);
        }
    }
    delete pm;
    return result;
}

inline std::vector<std::vector<int>> partitionForestToPathByMatching(int partition_idx) {
    TREE new_forest = origin_forest;
    // initalize the father arrays
    int components_num = component_from_spanning_forest.size();
    for (int i = 0; i < components_num; i++) {
        father[i] = i;
    }

    std::vector<TreeEdgeConnect> tree_edge_connects;
    int tot_edges = 0;
    int partitions_size = partitions[partition_idx].size();
    for (int i = 0; i < partitions_size; i++) {
        const auto& vec = partitions[partition_idx][i];
        int siz = vec.size();
        tot_edges += siz - 1;
        if (siz == 1) continue;
        // we need to connect all nodes in vec
        for (int com_u_vid = 0; com_u_vid < siz; com_u_vid++) {
            int com_u = vec[com_u_vid];
            for (int com_v_vid = 0; com_v_vid < siz; com_v_vid++) {
                if (com_u_vid == com_v_vid) continue;
                int com_v = vec[com_v_vid];
                // find the minimum cost edge between component u and component v
                tree_edge_connects.push_back(getMinTreeEdge(com_u, com_v));
            }
        }
    }
    // kruskal to combine the trees
    std::sort(tree_edge_connects.begin(), tree_edge_connects.end(), cmpTreeEdgeConnect);
    int add_edges = 0;
    for (int i = 0; i < tree_edge_connects.size(); i++) {
        int com_u = tree_edge_connects[i].com_u;
        int com_v = tree_edge_connects[i].com_v;
        int fa_u = find_father(com_u);
        int fa_v = find_father(com_v);
        if (fa_u != fa_v) {
            father[fa_v] = fa_u;
            // add edge to new forest
            int u = tree_edge_connects[i].u;
            int v = tree_edge_connects[i].v;
            new_forest.adj[u].push_back(v);
            new_forest.adj[v].push_back(u);
            new_forest.totcost += tree_edge_connects[i].cost;
            add_edges++;
        }
    }
    // to ensure there ia still a forest
    if (add_edges != tot_edges) {
        throw std::runtime_error("Error in forming new forest from partition!");
    }

    std::vector<int> odds;
    std::vector<int> degree(vertexnum, 0);
    for (int i = 0; i < vertexnum; ++i) {
        for (int j : new_forest.adj[i]) {
            assert(i != j);
            degree[j]++;
        }
    }
    int odd_cnt = 0;
    for (int i = 0; i < vertexnum; ++i) {
        if (degree[i] & 1) {
            odd_cnt++;
            odds.push_back(i);
        }
    }

    assert(odd_cnt % 2 == 0);

    auto match_edge = MinimumCostPerfectMatching(odds);
    for (const auto& edge : match_edge) {
        int u = edge.u;
        int v = edge.v;
        new_forest.adj[u].push_back(v);
        new_forest.adj[v].push_back(u);
    }

    // test
    std::vector<int> degree_test(vertexnum, 0);
    for (int i = 0; i < vertexnum; ++i) {
        for (int j : new_forest.adj[i]) {
            degree_test[j]++;
        }
    }
    for (int i = 0; i < vertexnum; ++i) {
        if (degree_test[i] % 2 == 1) {
            throw std::runtime_error("matching is wrong.\n");
        }
    }

    std::vector<std::vector<int>> pathsFromForest;
    unordered_set<int> unique_nodes;
    int customer_num = 0;
    for (int i = 0; i < depotsnum; i++) {
        if (1) {
            std::vector<int> walk, vec;
            // dfsEuler(i, new_forest.adj, walk);
            getEulerPath(i, new_forest.adj, walk);
            // erase the same nodes
            if (walk.size() <= 1) continue;
            // cout << '\n';
            for (const auto& node : walk) {
                // std::cout << node << "->";
                if (!unique_nodes.count(node) && node >= depotsnum) {
                    vec.push_back(node);
                    customer_num++;
                }
                unique_nodes.insert(node);
            }
            // cout << '\n';
            pathsFromForest.push_back(vec);
        }
    }
    if (customer_num != customersnum) {
        throw std::runtime_error("Error in path constructing!");
    }
    return pathsFromForest;
}

inline void computeResultByMinCostPerfectMatching() {

    int partitions_size = partitions.size();
    int components_num = component_from_spanning_forest.size();
    father.resize(components_num);
    double min_cost = INFINITE;
    std::vector<int> assignments;

    int cnt = 0;
    for (int i = 0; i < partitions_size; ++i) {
        if (GlobalTimer::isTimeOut()) {
            break;
        }
        auto euler_paths_from_forest = partitionForestToPathByMatching(i);

        // printPathsFromForest(euler_paths_from_forest);

        auto demand_assignment_paths = cycleSpiltToPaths(euler_paths_from_forest);

        // this scheme needs more vehicles
        if (demand_assignment_paths.size() == 0) {
            std::cout << "partition " << i << " can not be satify! \n";
            continue;
        }

        printDemandAssignmentPaths(demand_assignment_paths);

        auto depots_to_paths = minCostMaxFlow(demand_assignment_paths);

        auto printDepotToPath = [&](int x) {
            for (const auto& flow_sol : depots_to_paths) {
                int u = flow_sol.u;
                int v_id = flow_sol.v;
                int flow = flow_sol.flow;
                int v = v_id - depotsnum;
                cout << "depot is : " << u << " to choose the demand assignment path " << v << '\n';
                cout << "The path is : " << '\n';
                for (const auto& [customer_id, demand] : demand_assignment_paths[v].path) {
                    cout << "( " << customer_id << " , " << demand << ")  ";
                }
                cout << "the cnt_path is: " << flow;
                cout << '\n';
            }
        };
        // printDepotToPath(0);

        initRoute();
        double cost = 0;
        std::vector<int> assignmentvehicles;
        for (const auto& flow_sol : depots_to_paths) {
            int depot_id = flow_sol.u;
            int v_id = flow_sol.v;
            int flow = flow_sol.flow;

            auto path = demand_assignment_paths[v_id].path;
            cost += getMinCostRoute(depot_id, path) * flow;
            assignmentvehicles.push_back(flow);
        }
        // std::cout << "\n\n\n";
        // std::cout << "The tot cost is " << cost << '\n';
        // printRoute(assignmentvehicles);

        if (min_cost - cost > 1e-3) {
            min_cost = cost;
            MinCostRoute = TmpRoute;
            assignments = assignmentvehicles;
        }
        cnt++;
    }
    std::cout << "compute rounds : " << cnt << std::endl;
    std::cout << std::fixed << std::setprecision(10) << std::noshowpoint;
    std::cout << "\n\n\nThe minimum cost route is : " << min_cost << '\n';
    printMinRoute(assignments);
}

// *************************************************************
// using the GW algorithm to get the path

inline std::vector<std::vector<int>> DepotsDemands;
inline int dfs_cnt = 0;
inline int q_mod;
inline int Q;
inline int remain_capacity;
inline int idx1;
inline int k1;
inline int current_sum1;
inline void dfs(int idx, int k, int current_sum, std::vector<int>& path) {
    if (dfs_cnt >= DEPOT_ENUMS_LIMIT) {
        idx1 = idx;
        k1 = k;
        current_sum1 = current_sum;
        return;
    }

    if (idx == k - 1) {
        int current_mod = (current_sum + q_mod) % Q;
        int last_val = (current_mod == 0) ? 0 : (Q - current_mod);
        if (current_sum + last_val <= remain_capacity) {
            path.push_back(last_val);
            DepotsDemands.push_back(path);
            dfs_cnt++;
            path.pop_back();
        }
        return;
    }

    for (int i = 0; i < Q; ++i) {
        if (current_sum + i > remain_capacity) {
            break;
        }
        path.push_back(i);
        dfs(idx + 1, k, current_sum + i, path);
        path.pop_back();

        if (dfs_cnt >= DEPOT_ENUMS_LIMIT) {
            idx1 = idx;
            k1 = k;
            current_sum1 = current_sum;
            return;
        }
    }
}

inline void printDepotMod() {
    for (int i = 0; i < DepotsDemands.size(); ++i) {
        int sum = 0;
        for (auto j : DepotsDemands[i]) {
            // std::cout << j << ' ';
            sum += j;
        }
        if ((sum + q_mod) % Q != 0) {
            throw std::runtime_error("Depot Demand assign error with depot " + std::to_string(i));
        }
        // std::cout << '\n';
    }
}

inline std::vector<Cycles> GWCycleSpiltToPaths(const std::vector<std::vector<int>>& cycle,
                                               const std::vector<int>& demand_with_depots) {
    std::vector<Cycles> paths;
    int Q = info.vehicle_capacity;
    std::vector<int> copy_demands = demand_with_depots;

    std::vector<DemandAssignmentToCustomer> path;
    int need_vehicles = 0;
    int all_demands = 0;
    // if demand of a customer >= Q, split it first
    // one node is define as a trival path
    for (int i = 0; i < copy_demands.size(); i++) {
        if (copy_demands[i] >= Q) {
            int full_load_times = copy_demands[i] / Q;
            int remaining_demand = copy_demands[i] % Q;

            all_demands += copy_demands[i] - remaining_demand;

            int customer_id = i;

            DemandAssignmentToCustomer dac;
            dac.customer = customer_id;
            dac.assignmentdemand = Q;
            path.push_back(dac);
            Cycles new_cycle;
            new_cycle.path = path;
            new_cycle.cnt_path = full_load_times;
            paths.push_back(new_cycle);

            path.clear();
            // there exist demand for customer i
            copy_demands[i] = remaining_demand;
            need_vehicles += full_load_times;
        }
    }

    for (const auto& cyc : cycle) {
        int current_load = 0;

        for (const auto& customer : cyc) {
            // we skip the depot's demand

            int demand = copy_demands[customer];

            if (demand == 0 && customer >= depotsnum) continue;

            if (current_load + demand <= Q) {
                DemandAssignmentToCustomer dac;
                dac.customer = customer;
                dac.assignmentdemand = demand;
                path.push_back(dac);

                current_load += demand;
                if (customer >= depotsnum) {
                    all_demands += demand;
                }
                copy_demands[customer] = 0;
                if (current_load == Q) {
                    Cycles tmp_cycle;
                    tmp_cycle.path = path;
                    tmp_cycle.cnt_path = 1;
                    paths.push_back(tmp_cycle);
                    path.clear();
                    current_load = 0;
                }
            } else {
                if (customer >= depotsnum) {
                    all_demands += demand;
                }
                int remaining_capacity = Q - current_load;
                // fill the remaining capacity
                DemandAssignmentToCustomer dac;
                dac.customer = customer;
                dac.assignmentdemand = remaining_capacity;
                path.push_back(dac);
                Cycles tmp_cycle;
                tmp_cycle.path = path;
                tmp_cycle.cnt_path = 1;
                paths.push_back(tmp_cycle);
                path.clear();
                if (demand - remaining_capacity > 0) {
                    current_load = demand - remaining_capacity;
                    // update the demand of this customer
                    dac.assignmentdemand = current_load;
                    dac.customer = customer;
                    path.push_back(dac);
                } else {
                    current_load = 0;
                }
                copy_demands[customer] = 0;
            }
        }
        // we construct the paths' demand are all mod Q equals zero
        if (path.size() > 0) {
            for (auto it : path) {
                if (it.assignmentdemand > 0) {
                    throw std::runtime_error("cycle split error\n");
                }
            }
        }
    }
    if (all_demands != info.tot_demand) {
        throw std::runtime_error("cycle split error!!!!\n");
    }
    return paths;
}

inline void computeResultByGW() {
    double min_cost = INFINITE;
    std::vector<int> assignments;
    int cnt = 0;
    idx1 = 0;
    current_sum1 = 0;
    std::vector<int> temp_path;
    while (!GlobalTimer::isTimeOut()) {
        bool flag = false;
        std::vector<int> demand_with_depots(vertexnum);
        int customer_demand = 0;
        Q = info.vehicle_capacity;
        for (int i = 0; i < info.customer_num; ++i) {
            customer_demand += info.demands[i];
            demand_with_depots[i + depotsnum] = info.demands[i];
        }

        q_mod = customer_demand % info.vehicle_capacity;
        remain_capacity = info.vehicle_capacity * info.vehicle_num - customer_demand;

        for (int i = 0; i < DepotsDemands.size(); ++i) {
            DepotsDemands[i].clear();
        }
        int k = info.depots_loc.size();
        // cout << "q_mod is: " << q_mod << '\n';
        cout << "remain_capacity is: " << remain_capacity << '\n';

        dfs(idx1, k, current_sum1, temp_path);
        printDepotMod();

        auto depotDemandCombine = [&](int idx) {
            for (int i = 0; i < depotsnum; ++i) {
                demand_with_depots[i] = DepotsDemands[idx][i];
            }
        };

        auto checkDepotWithCustomerDemand = [&](int idx) {
            int sum = 0;
            for (int i = 0; i < vertexnum; ++i) {
                sum += demand_with_depots[i];
            }
            if (sum % Q != 0) {
                throw std::runtime_error(std::to_string(idx) + "the demand is error!\n");
            }
        };

        auto checkPathFindingByGW = [&](const std::vector<std::vector<int>>& tmp) {
            std::unordered_map<int, int> mp;

            // cout << "GW Paths \n";
            for (int i = 0; i < tmp.size(); ++i) {
                int sum = 0;
                // cout << "path : " << i << "  ";
                for (int j : tmp[i]) {
                    if (mp.count(j)) {
                        throw std::runtime_error(std::to_string(i) + "Path error!\n");
                    }
                    mp[j] = 1;
                    // cout << j << ' ';
                    sum += demand_with_depots[j];
                }
                if (sum % Q != 0) {
                    throw std::runtime_error("GW is error\n");
                }
                // cout << '\n';
            }
            if (mp.size() != vertexnum) {
                throw std::runtime_error("Path error!\n");
            }
        };

        for (int i = 0; i < DepotsDemands.size(); ++i) {
            if (GlobalTimer::isTimeOut()) {
                flag = true;
                break;
            }

            depotDemandCombine(i);

            checkDepotWithCustomerDemand(i);
            auto euler_paths_from_GW = GW(demand_with_depots, Q);
            checkPathFindingByGW(euler_paths_from_GW);

            auto demand_assignment_paths = GWCycleSpiltToPaths(euler_paths_from_GW, demand_with_depots);

            printDemandAssignmentPaths(demand_assignment_paths);

            auto depots_to_paths = minCostMaxFlow(demand_assignment_paths);
            auto printDepotToPath = [&](int x) {
                for (const auto& flow_sol : depots_to_paths) {
                    int u = flow_sol.u;
                    int v_id = flow_sol.v;
                    int flow = flow_sol.flow;
                    int v = v_id - depotsnum;
                    cout << "depot is : " << u << " to choose the demand assignment path " << v << '\n';
                    cout << "The path is : " << '\n';
                    for (const auto& [customer_id, demand] : demand_assignment_paths[v].path) {
                        cout << "( " << customer_id << " , " << demand << ")  ";
                    }
                    cout << "the cnt_path is: " << flow;
                    cout << '\n';
                }
            };

            initRoute();
            double cost = 0;
            std::vector<int> assignmentvehicles;
            for (const auto& flow_sol : depots_to_paths) {
                int depot_id = flow_sol.u;
                int v_id = flow_sol.v;
                int flow = flow_sol.flow;

                int v = v_id;
                auto path = demand_assignment_paths[v].path;
                cost += getMinCostRoute(depot_id, path) * flow;
                assignmentvehicles.push_back(flow);
            }
            // std::cout << "\n\n\n";
            // std::cout << "The tot cost is " << cost << '\n';
            // printRoute(assignmentvehicles);

            if (min_cost - cost > 1e-3) {
                min_cost = cost;
                MinCostRoute = TmpRoute;
                assignments = assignmentvehicles;
            }
            cnt++;
            // std::cout << "Round " << i << " is finish. The cost is : " << cost <<" \n";;
        }
        if (flag || DepotsDemands.size() < DEPOT_ENUMS_LIMIT) {
            break;
        }
    }

    std::cout << "valid solution is : " << cnt << std::endl;
    std::cout << std::fixed << std::setprecision(10) << std::noshowpoint;
    std::cout << "\n\n\nThe minimum cost route is : " << min_cost << '\n';
    printMinRoute(assignments);
}
