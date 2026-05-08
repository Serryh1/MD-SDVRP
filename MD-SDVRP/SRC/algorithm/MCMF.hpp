#pragma once

#include <lemon/cost_scaling.h>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>

#include <cassert>
#include <cmath>  
#include <limits>
#include <vector>

#include "../structure/DSDVRP_function.hpp"
#include "../utils/SpanningForest.hpp"

using namespace lemon;

const double COST_SCALE = 100.0;

inline std::vector<Flow> minCostMaxFlow(const std::vector<Cycles>& tmp_cycles) {
    typedef ListDigraph Graph;
    typedef long long FlowType;
    typedef long long CostType;

    Graph g;

    Graph::NodeMap<int> node_id(g);
    Graph::NodeMap<int> node_type(g);  // 1: Depot, 2: Path

    Graph::Node s = g.addNode();
    Graph::Node t = g.addNode();

    std::vector<Graph::Node> depot_nodes;
    for (int i = 0; i < depotsnum; ++i) {
        Graph::Node n = g.addNode();
        depot_nodes.push_back(n);
        node_id[n] = i;
        node_type[n] = 1;
    }

    std::vector<Graph::Node> path_nodes;
    for (size_t i = 0; i < tmp_cycles.size(); ++i) {
        Graph::Node n = g.addNode();
        path_nodes.push_back(n);
        node_id[n] = (int)i;
        node_type[n] = 2;
    }

    Graph::ArcMap<FlowType> arc_capacity(g);
    Graph::ArcMap<CostType> arc_cost(g);

    auto addEdge = [&](Graph::Node u, Graph::Node v, FlowType cap, double raw_cost) {
        Graph::Arc arc = g.addArc(u, v);
        arc_capacity[arc] = cap;
        arc_cost[arc] = static_cast<CostType>(std::round(raw_cost * COST_SCALE));
    };

    // Source -> Depots
    for (int i = 0; i < depotsnum; ++i) {
        FlowType vehicle_count = info.vehicles[i];
        addEdge(s, depot_nodes[i], vehicle_count, 0.0);
    }

    auto getMinDistanceFromDepotToPath = [&](int depot_idx, int path_idx) {
        auto& path = tmp_cycles[path_idx].path;
        double min_dist = std::numeric_limits<double>::max();
        Position depot_pos = info.depots_loc[depot_idx];
        for (const auto& customer : path) {
            Position customer_pos = info.all_loc[customer.customer];
            double dist = GetEdgeVal(depot_pos, customer_pos);
            if (dist < min_dist) {
                min_dist = dist;
            }
        }
        return min_dist;
    };

    // Depots -> Paths
    int cycle_size = (int)tmp_cycles.size();
    for (int i = 0; i < depotsnum; ++i) {
        for (int j = 0; j < cycle_size; ++j) {
            FlowType flow = tmp_cycles[j].cnt_path;
            double dist = getMinDistanceFromDepotToPath(i, j);

            double raw_edge_cost = 0.0;
            if (flow == 1)
                raw_edge_cost = dist * flow;
            else
                raw_edge_cost = 2 * dist * flow;

            addEdge(depot_nodes[i], path_nodes[j], flow, raw_edge_cost);
        }
    }

    // Paths -> Sink
    for (int j = 0; j < cycle_size; ++j) {
        FlowType flow = tmp_cycles[j].cnt_path;
        addEdge(path_nodes[j], t, flow, 0.0);
    }
    //  (Max Flow)
    Preflow<Graph, Graph::ArcMap<FlowType>> preflow_algo(g, arc_capacity, s, t);
    preflow_algo.run();

    FlowType max_flow_val = preflow_algo.flowValue();
    // std::cout << "Calculated Max Flow: " << max_flow_val << std::endl;

    //  (Cost Scaling)

    // <Graph, Value(Flow), Cost>
    CostScaling<Graph, FlowType, CostType> cs(g);

    // 
    cs.upperMap(arc_capacity).costMap(arc_cost).stSupply(s, t, max_flow_val);

    CostScaling<Graph, FlowType, CostType>::ProblemType res = cs.run();

    std::vector<Flow> result;

    if (res == CostScaling<Graph, FlowType, CostType>::OPTIMAL) {
        Graph::ArcMap<FlowType> flow_map(g);
        cs.flowMap(flow_map);

        for (Graph::ArcIt a(g); a != INVALID; ++a) {
            if (flow_map[a] > 0) {
                Graph::Node u = g.source(a);
                Graph::Node v = g.target(a);

                //  Depot -> Path 
                if (node_type[u] == 1 && node_type[v] == 2) {
                    Flow f;
                    f.u = node_id[u];
                    f.v = node_id[v];
                    f.flow = flow_map[a];
                    // f.cost = (double)arc_cost[a] / COST_SCALE; 
                    result.push_back(f);
                }
            }
        }
    } else {
        std::cout << "Infeasible." << std::endl;
    }

    return result;
}