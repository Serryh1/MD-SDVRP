#pragma once


#include <iostream>
#include <unordered_map>
#include <vector>

#include "../structure/DSDVRP_function.hpp"
#include "../utils/SpanningForest.hpp"

const int MAX_LIMIT = 400000;  // the maximum partition rounds

// the origin components from spanning forest
inline std::vector<Component> component_from_spanning_forest;
// all the permutation for all the components
inline std::vector<std::vector<std::vector<int>>> partitions;

inline void spanningForestToComponent() {
    std::vector<int> vec;
    std::unordered_map<int, bool> visited;
    auto dfs = [&](int u, int fa, auto&& dfs_ref) -> void {
        vec.push_back(u);
        visited[u] = true;
        for (const auto& v : origin_forest.adj[u]) {
            if (!visited[v]) {
                dfs_ref(v, u, dfs_ref);
            }
        }
    };

    for (int i = 0; i < depotsnum; ++i) {
        Component comp;
        comp.root = i;
        vec.clear();
        dfs(i, -1, dfs);
        if ((int)vec.size() == 1) continue;  // only depot no customers
        for (const auto& node : vec) {
            comp.nodes.push_back(node);
        }
        component_from_spanning_forest.push_back(comp);
    }
    return;
}



inline std::vector<int> nums;
inline int total_ways = 0;
inline std::vector<std::vector<int>> temp_partition;

// to solve all the permutation for the components
// the upper bound for permutation round is MAX_LIMIT(= 10000)
inline void dfs(int index) {
    if (total_ways >= MAX_LIMIT) {
        return;
    }

    if (index == nums.size()) {
        partitions.push_back(temp_partition);
        total_ways++;
        return;
    }

    int current_val = nums[index];

    for (int i = 0; i < temp_partition.size(); ++i) {
        // optimization:
        if (total_ways >= MAX_LIMIT) return;

        temp_partition[i].push_back(current_val);
        dfs(index + 1);
        temp_partition[i].pop_back();
    }

    // 2: create a new group
    if (total_ways >= MAX_LIMIT) return;

    temp_partition.push_back({current_val});
    dfs(index + 1);
    temp_partition.pop_back();
}

inline void printPartitions() {
    std::cout << "Total partitions: " << partitions.size() << std::endl;
    for (const auto& temp_partition : partitions) {
        for (const auto& vec : temp_partition) {
            std::cout << "{ ";
            for (const auto& val : vec) {
                std::cout << val << " ";
            }
            std::cout << "} ";
        }
        std::cout << std::endl;
    }
}

inline void getPartitions() {
    spanningForestToComponent();
    int num = component_from_spanning_forest.size();
    for (int i = 0; i < num; ++i) {
        nums.push_back(i);
    }
    dfs(0);
    // std::cout << partitions.size() << std::endl;
    // printPartitions();
}
