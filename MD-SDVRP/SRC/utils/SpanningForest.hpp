#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "../structure/DSDVRP_function.hpp"
#include "../utils/DataProcess.hpp"

using namespace std;
#define INFINITE 0xFFFFFFF

inline int vertexnum;
inline int depotsnum;
inline int customersnum;
inline TREE origin_forest;
inline std::unordered_map<int,int> odd_nodes_in_origin_forest;

// calculate the Euclidean distance between two positions
inline double GetEdgeVal(const Position& a, const Position& b) {
    return sqrt(1.0 * ((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)));
}

// initialize vertices including depots and customers
inline std::vector<CITIES> customer_and_depot_to_city(const Info& info) {
    std::vector<CITIES> vertices;
    CITIES city;

    for (int i = 0; i < info.depots_loc.size(); i++) {
        city.x = info.depots_loc[i].x;
        city.y = info.depots_loc[i].y;
        city.demand = 0;
        vertices.push_back(city);
    }

    for (int i = 0; i < info.customers_loc.size(); i++) {
        city.x = info.customers_loc[i].x;
        city.y = info.customers_loc[i].y;
        city.demand = info.demands[i];
        vertices.push_back(city);
    }

    return vertices;
}

inline std::vector<std::vector<double>> adjMatrix;
// adjacency matrix for all vertices  vertexnum x vertexnum
inline std::vector<std::vector<double>> matrixWithDepot;
// adjacency matrix for all customers and a synthetic depot(customer_num +1)

inline std::vector<int> vertex_to_depot;  // map each vertex to its nearest depot

// initialize adjacency matrix for all vertices
inline void Matrix(const std::vector<CITIES>& vertices) {
    adjMatrix.resize(vertexnum, std::vector<double>(vertexnum, INFINITE));

    for (int i = 0; i < vertexnum; i++) {
        for (int j = 0; j < vertexnum; j++) {
            CITIES city1 = vertices[i];
            CITIES city2 = vertices[j];
            adjMatrix[i][j] = GetEdgeVal(Position{city1.x, city1.y}, Position{city2.x, city2.y});
        }
    }
}

// solve the nearest depot for each customer
inline std::vector<double> minDepots() {
    std::vector<double> vertex_to_nearest_depot_distance;

    for (int i = 0; i < vertexnum; ++i) {
        // initial min_dist as distance to depot 0
        double min_dist = adjMatrix[i][0];
        int index = 0;
        for (int j = 1; j < depotsnum; ++j) {
            if (adjMatrix[i][j] < min_dist) {
                min_dist = adjMatrix[i][j];
                index = j;
            }
        }
        vertex_to_nearest_depot_distance.push_back(min_dist);
        vertex_to_depot.push_back(index);
    }
    return vertex_to_nearest_depot_distance;
}

// all depot id define as the node customer
inline void MultiDepotsToMatrix() {
    int num = customersnum;
    std::vector<double> vertex_to_nearest_depot_distance = minDepots();

    matrixWithDepot.resize(num + 1, std::vector<double>(num + 1, INFINITE));

    for (int i = 0; i < num; ++i) {
        for (int j = 0; j < num; ++j) {
            matrixWithDepot[i][j] = adjMatrix[i + depotsnum][j + depotsnum];
        }
    }
    // each customer or depot to its nearest depot
    for (int i = 0; i < num; ++i) {
        matrixWithDepot[i][num] = vertex_to_nearest_depot_distance[i + depotsnum];
        matrixWithDepot[num][i] = vertex_to_nearest_depot_distance[i + depotsnum];
    }
}

// for edge sort
inline bool cmpEdge(const Edge& a, const Edge& b) { return a.cost < b.cost; }

inline std::vector<int> parent;
inline int unoin_find(int x) {
    if (parent[x] != x) {
        parent[x] = unoin_find(parent[x]);
    }
    return parent[x];
}

inline TREE Kruskal_depots() {
    TREE spanning_tree;
    spanning_tree.adj.resize(vertexnum, std::vector<int>(vertexnum));

    for (int i = 0; i < vertexnum; ++i) {
        for (int j = 0; j < vertexnum; ++j) {
            spanning_tree.adj[i][j] = 0;
            spanning_tree.totcost += 0.0;
        }
    }

    std::vector<Edge> edges;
    
    int num = customersnum + 1;
    for (int i = 0; i < num; ++i) {
        for (int j = i + 1; j < num; ++j) {
            if (matrixWithDepot[i][j] < INFINITE) {
                Edge edge;
                edge.u = i;
                edge.v = j;
                edge.cost = matrixWithDepot[i][j];
                edges.push_back(edge);
            }
        }
    }
    // sort edges by cost
    std::sort(edges.begin(), edges.end(), cmpEdge);
    // initialize union-find structure
    for (int i = 0; i < vertexnum; ++i) {
        parent.push_back(i);
    }
    // for(int i = 0; i < edges.size(); ++i) {
    //     std::cout << edges[i].u << "---" << edges[i].v << " : " << edges[i].cost << std::endl;
    // }

    for (int i = 0; i < edges.size(); ++i) {
        int u = edges[i].u;
        int v = edges[i].v;
        int u_root = unoin_find(u);
        int v_root = unoin_find(v);
        if (u_root != v_root) {
            parent[v_root] = u_root;
            // add edge to spanning tree
            spanning_tree.adj[u][v] = 1;
            spanning_tree.totcost += edges[i].cost;
        }
    }
    return spanning_tree;
}

inline void printTreeDepots(const TREE& t) {
    std::cout << "Spanning Tree Edges: " << std::endl;
    for (int i = 0; i < customersnum + 1; i++) {
        for (int j = 0; j < customersnum + 1; j++) {
            if (t.adj[i][j] == 1) {
                std::cout << i << "--->" << j << std::endl;
            }
        }
    }
}

inline void printForecast(const TREE& t) {
    std::cout << "Spanning Forest Edges: " << std::endl;
    for (int i = 0; i < t.adj.size(); ++i) {
        const auto& vec = t.adj[i];
        for (const auto& val : vec) {
            std::cout << i << "->" << val << '\n';
        }
    }
}

inline TREE TreetoForest(TREE t) {
    TREE forest;
    forest.adj.resize(vertexnum);
    forest.totcost = 0.0;

    std::vector<Edge> edges;
    
    // spilt the node n to each depots
    int n = customersnum;
    for (int i = 0; i < n; ++i) {
        if (t.adj[i][n] == 1) {
            Edge edge;
            edge.u = i + depotsnum;
            edge.v = vertex_to_depot[i + depotsnum];
            edge.cost = t.totcost;
            edges.push_back(edge);
        }
    }

    for (int i = 0; i < edges.size(); ++i) {
        int u = edges[i].u;
        int v = edges[i].v;
        // assure the root is the depot
        if (u < v) {
            forest.adj[u].push_back(v);
            forest.adj[v].push_back(u);
            forest.totcost += edges[i].cost;
        } else {
            forest.adj[v].push_back(u);
            forest.adj[u].push_back(v);
            forest.totcost += edges[i].cost;
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            if (t.adj[i][j] == 1) {
                forest.adj[i + depotsnum].push_back(j + depotsnum);
                forest.adj[j + depotsnum].push_back(i + depotsnum);
                forest.totcost += t.totcost;
            }
        }
    }
    return forest;
}

inline void getOddNodes(){
    const auto &vec = origin_forest.adj;
    std::vector<int> degrees(vertexnum, 0);
    for(int i = 0; i < vertexnum; ++ i){
        for(const auto &v: vec[i]){
            if(v != i){
                degrees[v] ++;
            }
        }
    }
    int count_odd_nodes = 0;
    for(int i = 0; i < vertexnum; ++ i){
        if(degrees[i] & 1){
            odd_nodes_in_origin_forest[i] = 1;
            count_odd_nodes ++;
        }
    }
    if(count_odd_nodes % 2 != 0){
        throw std::runtime_error("the origin forest construct error\n");
    }
}

inline void spanningForest() {
    std::vector<CITIES> vertices = customer_and_depot_to_city(info);
    vertexnum = (int)vertices.size();
    depotsnum = info.depots_num;
    customersnum = info.customer_num;

    Matrix(vertices);
    MultiDepotsToMatrix();

    TREE spanning_tree;
    spanning_tree = Kruskal_depots();
    // printTreeDepots(spanning_tree);

    origin_forest = TreetoForest(spanning_tree);
    // printForecast(origin_forest);

    // getOddNodes();
    
}
