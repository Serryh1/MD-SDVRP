#pragma once

#include <algorithm>
#include <cassert>
#include <map>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../algorithm/SF-solution.hpp"
#include "../utils/SpanningForest.hpp"

class edges {
   public:
    double edgeWeight;
    std::pair<int, int> edgeCoordinates;
};

inline bool operator<(const edges& lhs, const edges& rhs) { return lhs.edgeWeight < rhs.edgeWeight; }

inline bool operator>(const edges& lhs, const edges& rhs) { return lhs.edgeWeight > rhs.edgeWeight; }

inline bool operator==(const edges& lhs, const edges& rhs) {
    return (lhs.edgeCoordinates.first == rhs.edgeCoordinates.first) ||
           (lhs.edgeCoordinates.second == rhs.edgeCoordinates.first);
}

inline std::vector<int> fa;
inline std::vector<int> capacity;
inline std::vector<int> Rank;
inline std::vector<int> isactive;
inline int union_find(int x) {
    if (fa[x] != x) fa[x] = union_find(fa[x]);
    return fa[x];
}

inline void Unite(int x, int y, int Q) {
    int px = union_find(x);
    int py = union_find(y);
    if (px == py) return;

    if (Rank[px] < Rank[py]) {
        fa[px] = py;
        capacity[py] += capacity[px];
        if (capacity[py] % Q == 0) {
            isactive[py] = 0;
        } else {
            isactive[py] = 1;
        }
    } else if (Rank[px] > Rank[py]) {
        fa[py] = px;
        capacity[px] += capacity[py];
        if (capacity[px] % Q == 0) {
            isactive[px] = 0;
        } else {
            isactive[px] = 1;
        }
    } else {
        fa[py] = px;
        capacity[px] += capacity[py];
        if (capacity[px] % Q == 0) {
            isactive[px] = 0;
        } else {
            isactive[px] = 1;
        }
        Rank[px]++;
    }
}

struct Components {
    std::vector<int> nodes;
    std::vector<std::pair<int, int>> edges;
    std::vector<int> degree;
    double dual = 0.0;
    bool frozen = false;

    Components(int n, int id) : degree(n, 0) { nodes.push_back(id); }
};

struct GWEdge {
    double key;
    int u, v;
    int label;
    GWEdge(double k, int u, int v, int label) : key(k), u(u), v(v), label(label) {}

    bool operator>(const GWEdge& other) const { return key > other.key; }
};

struct Newedge {
    // Component (id, node)
    std::pair<int, int> Componentu, Componentv;
    int label;

    bool friend operator<(Newedge x, Newedge y) { return x.Componentv.first < y.Componentv.first; };
};

inline std::vector<std::vector<int>> GW(const std::vector<int>& copy_demand_with_depot, int Q) {
    std::vector<std::vector<int>> QPaths;

    steinerForest solutionGraph(vertexnum);  // Create the solution graph
    solutionGraph.Path.clear();              // path
    solutionGraph.visit.resize(vertexnum, false);

    std::vector<double> d(vertexnum);
    // union initial
    fa.resize(vertexnum);
    capacity.resize(vertexnum);
    Rank.resize(vertexnum, 0);
    isactive.resize(vertexnum);

    int n = vertexnum;
    for (int i = 0; i < n; ++i){
        fa[i] = i;
        capacity[i] = copy_demand_with_depot[i];
        solutionGraph.node_values.push_back(capacity[i]);
    } 
    for (int i = 0; i < n; ++i) {
        if (capacity[i] % Q == 0) {
            isactive[i] = false;
        }else{
            isactive[i] = true;
        }
    }

    // store the edges
    std::vector<pair<int, int>> solutionsEdges;
    std::priority_queue<GWEdge, std::vector<GWEdge>, std::greater<GWEdge>> pq;

    std::vector<std::vector<Newedge>> new_Edges(n);

    // for each components build an edge set <v, label>
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dist;
            if(isactive[i] && isactive[j]){
                dist = 1e9;
            }else if(isactive[i] || isactive[j]){
                dist = adjMatrix[i][j];
            }else {
                dist = adjMatrix[i][j] / 2;
            }
            pq.push(GWEdge(dist, i, j, 0));
            std::pair<int, int> C_u = {i, i};
            std::pair<int, int> C_v = {j, j};
            new_Edges[i].push_back({C_u, C_v, 0});
            new_Edges[j].push_back({C_v, C_u, 0});
        }
        sort(new_Edges[i].begin(), new_Edges[i].end());
    }

    bool flag = false;
    // check there is no active nodes
    std::tuple<int, int, int> tp;

    // restore the deleted edges
    std::map<std::tuple<int, int, int>, int> LazyPQ;
    double last_key = 0;
    int idx = 0;
    // the label idx
    while (!pq.empty() && !flag) {
        idx++;
        const auto [key, u, v, label] = pq.top();
        pq.pop();
        // if this edge has been deleted, then continue
        if (LazyPQ.count(make_tuple(u, v, label))) continue;
        int Cp = union_find(u), Cq = union_find(v);
        // if the two components are inactive or they belong to the same component, then continue
        if (Cp == Cq || (!isactive[Cp] && !isactive[Cq])) continue;

        solutionGraph.addEdge(u, v, adjMatrix[u][v]);
        solutionsEdges.push_back({u, v});

        double epsilon = key - last_key;
        last_key = key;
        // undate all active nodes
        for (int i = 0; i < n; ++i) {
            int p_i = union_find(i);
            if (isactive[p_i]) {
                d[i] += epsilon;
            }
        }

        // union components
        if ((capacity[Cp] + capacity[Cq]) % Q == 0) {
            isactive[Cp] = 0;
            isactive[Cq] = 0;
        } else {
            isactive[Cp] = 1;
            isactive[Cq] = 1;
        }

        // delete edges

        int size_cp = new_Edges[Cp].size();
        int size_cq = new_Edges[Cq].size();
        int j = 0;

        std::vector<Newedge> temp;

        for (int i = 0; i < size_cp; ++i) {
            const auto& [ComponentP, ComponentPR, labelp] = new_Edges[Cp][i];
            while (j < size_cq && new_Edges[Cq][j].Componentv.first < ComponentPR.first) {
                j++;
            }
            if (j == size_cq) continue;
            auto [ComponentQ, ComponentQR, labelq] = new_Edges[Cq][j];
            // beglongs to the same

            int pu = ComponentP.second;
            int pr = ComponentPR.second;
            int qu = ComponentQ.second;
            int qr = ComponentQR.second;
            if (ComponentPR.first != ComponentQR.first) continue;
            // delete two edges
            LazyPQ[make_tuple(pu, pr, labelp)] = 1;
            LazyPQ[make_tuple(qu, qr, labelq)] = 1;
            double key_p = key;
            double key_q = key;

            int Cr = ComponentPR.first;

            int activepr = isactive[Cp] + isactive[Cr];
            int activeqr = isactive[Cq] + isactive[Cr];

            if (!activepr && !activeqr) {
                double p_1 = adjMatrix[pu][pr] - d[pu] - d[pr];
                double p_2 = adjMatrix[qu][qr] - d[qu] - d[qr];

                if (p_1 - p_2 <= 1e-3) {
                    temp.push_back({ComponentP, ComponentPR, idx});
                    pq.push({1e9, pu, pr, idx});
                } else {
                    temp.push_back({ComponentQ, ComponentQR, idx});
                    pq.push({1e9, qu, qr, idx});
                }
                continue;
            }

            key_p += (adjMatrix[pu][pr] - d[pu] - d[pr]) / activepr;
            key_q += (adjMatrix[qu][qr] - d[qu] - d[qr]) / activeqr;

            // add back new min key edges
            if (key_p - key_q <= 1e-3) {
                temp.push_back({ComponentP, ComponentPR, idx});
                pq.push({key_p, pu, pr, idx});
            } else {
                temp.push_back({ComponentQ, ComponentQR, idx});
                pq.push({key_q, qu, qr, idx});
            }
        }
        new_Edges[Cp].clear();
        new_Edges[Cq].clear();

        Unite(Cp, Cq, Q);
        int C_new = union_find(Cp);
        sort(temp.begin(), temp.end());

        for (auto& [ComP, ComQ, label] : temp) {
            ComP.first = C_new;
            new_Edges[C_new].push_back({ComP, ComQ, label});
        }
    }

    // std::cout<<"Out of while loop \n";

    // BFS Clean process
    for(int i = 0; i < solutionsEdges.size(); ++ i){
        auto [u, v] = solutionsEdges[i];
        if(solutionGraph.BFSCleanSolution(u, v, Q)){
            solutionGraph.deleteEdge(u, v);
        }
    }

    std::unordered_set<int> unique_nodes;
    for(int i = 0; i < vertexnum; ++ i){
        if(!unique_nodes.count(i)){
            solutionGraph.Path.clear();
            solutionGraph.dfs(i);
            std::vector<int> temp_path;
            for(const auto &val: solutionGraph.Path){
                if(!unique_nodes.count(val)){
                    unique_nodes.insert(val);
                    temp_path.push_back(val);
                }
            }
            QPaths.push_back(temp_path);
        }
    }

    return QPaths;
}