//
// Created by DY on 2021/12/7.
//
#include <cstring>
// #include "miniSpanTree.h"
#include <math.h>

#include <algorithm>
#include <climits>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "blossom/PerfectMatching.h"

#define VertexData unsigned int
#define INFINITE 0xFFFFFFFF
#define M 4000
#define type 6  // type of depot: 2/4/6
int data_type;  // type of instance: SD=0, p/pr=1, g=2

// upper bound for combinations to avoid error
#define transfer_idx_limit 20000   // upper bound for permulation and combination
#define transfer_limit 2000        // upper bound for edge exchange times except single edge exchange
#define transfer_limit_all 100000  // upper bound for edge exchange times
// number of feasible solutions by Algorithm 2
#define add_limit 100000

using namespace std;
// const int vetex_number = 1518;  // num of vetex( = depots num + customer num )

string path_0_depots;  // instance file path
string path_1_depots;
string path_2_depots;

int vetex_number;


struct Info {
    int customer_num;
    int capacity;
    int depots_num;
    int vehicle_num;
    vector<int> vehicles;
    vector<int> demands;
    vector<vector<double>> loc;
};

typedef struct Tree {
    vector<vector<bool>> tree;
    double cost;

    void init(int n) {
        tree = vector<vector<bool>>(n, vector<bool>(n, 0));
        cost = 0;
    }
} TREE;

typedef struct TreeDepots {
    // bool tree[vetex_number - 1][vetex_number - 1];
    vector<vector<bool>> tree;
    double cost;

    void init(int n) {
        tree = vector<vector<bool>>(n - 1, vector<bool>(n - 1, 0));
        cost = 0;
    }

} TREE_DEPOTS;

typedef struct candidate {
    int x;  // X-coordinate
    int y;  // Y-coordinateX
    int d;  // demand
} CITIES;

// Edge Info
struct Arc {
    VertexData u;
    VertexData v;
    double cost;
    int arc_idx;
    bool operator<(Arc other) const {
        return arc_idx < other.arc_idx;  // 你的比较逻辑
    }
};

typedef struct Edge {
    int start;
    int end;
} EDGE;

Info readTxt() {
    Info info;
    ifstream infile;
    if (data_type == 1) {
        infile.open(path_1_depots, ios::in);
        if (!infile.is_open()) {
            cout << "can not open this file" << endl;
            return info;
        }
        string s;
        vector<vector<string>> res;
        while (getline(infile, s)) {
            stringstream input(s);
            string result;
            vector<string> in;
            while (input >> result) {
                in.push_back(result);
            }
            res.push_back(in);
        }
        info.customer_num = stoi(res[0][2]);  // customer number
        info.capacity = stoi(res[2][1]);      // capacity(assume it is same for each vehicle)
        info.depots_num = stoi(res[0][3]);    // depot number
        info.vehicle_num = info.depots_num * stoi(res[0][1]);
        for (int i = 0; i < info.depots_num; i++) {
            info.vehicles.push_back(stoi(res[0][1]));
        }

        int begin_line = info.depots_num + 1;
        for (int i = begin_line; i < info.customer_num + begin_line; i++) {
            info.demands.push_back(stoi(res[i][4]));
        }
        int demand = 0;
        for (int i = 0; i < info.demands.size(); i++) {
            demand += info.demands[i];
        }
        // location infos
        int len = info.depots_num + info.customer_num;
        for (int i = len + 1; i < len + 1 + info.depots_num; i++) {
            vector<double> xy;
            xy.push_back(stod(res[i][1]));
            xy.push_back(stod(res[i][2]));
            info.loc.push_back(xy);
        }
        for (int i = begin_line; i < begin_line + info.customer_num; i++) {
            vector<double> xy;
            xy.push_back(stod(res[i][1]));
            xy.push_back(stod(res[i][2]));
            info.loc.push_back(xy);
        }
    } else if (data_type == 0) {
        infile.open(path_0_depots, ios::in);
        if (!infile.is_open()) {
            cout << "can not open this file" << endl;
            return info;
        }
        string s;
        vector<vector<string>> res;
        while (getline(infile, s)) {
            stringstream input(s);
            string result;
            vector<string> in;
            while (input >> result) {
                in.push_back(result);
            }
            res.push_back(in);
        }
        info.customer_num = stoi(res[0][0]);  // customer number
        info.capacity = stoi(res[0][1]);      // capacity
        info.depots_num = res[0].size() - 2;  // depot number
        info.vehicle_num = 0;
        for (int i = 2; i < res[0].size(); i++) {
            info.vehicles.push_back(stoi(res[0][i]));
            info.vehicle_num = info.vehicle_num + stoi(res[0][i]);
        }  // vehiclle number of corresponding depots
        for (int i = 0; i < info.customer_num; i++) {
            info.demands.push_back(stoi(res[1][i]));
        }  // customer demand
        int len = info.depots_num + info.customer_num;
        for (int i = 2; i < len + 2; i++) {
            vector<double> xy;
            for (int j = 0; j < 2; j++) {
                xy.push_back(stod(res[i][j]));
            }
            info.loc.push_back(xy);
        }  // location infos
    } else {
        infile.open(path_2_depots, ios::in);
        int tot = 0;
        int need = 0;
        if (!infile.is_open()) {
            cout << "can not open this file" << endl;
            return info;
        }
        string s;
        vector<vector<string>> res;
        while (getline(infile, s)) {
            stringstream input(s);
            string result;
            vector<string> in;
            while (input >> result) {
                in.push_back(result);
            }
            res.push_back(in);
        }
        info.customer_num = stoi(res[0][0]);
        info.depots_num = stoi(res[0][1]);
        info.capacity = stoi(res[0][2]);
        info.vehicle_num = 0;
        for (int i = 1; i <= info.depots_num; ++i) {
            double x = stod(res[i][0]);
            double y = stod(res[i][1]);
            int veh = stoi(res[i][2]);
            vector<double> xy;
            xy.push_back(x);
            xy.push_back(y);
            info.loc.push_back(xy);
            info.vehicles.push_back(veh);
            info.vehicle_num += veh;
        }
        tot = info.capacity * info.vehicle_num;
        for (int i = info.depots_num + 1; i <= info.depots_num + info.customer_num; ++i) {
            double x = stod(res[i][0]);
            double y = stod(res[i][1]);
            int demand = stoi(res[i][2]);
            vector<double> xy;
            xy.push_back(x);
            xy.push_back(y);
            info.loc.push_back(xy);
            info.demands.push_back(demand);
            need += demand;
        }
        cout << "tot capacity is : " << tot << '\n';
        cout << "need capacity is : " << need << '\n';
    }
    infile.close();
    return info;
}

double get(vector<double> x, vector<double> y) {
    return sqrt((x[0] - y[0]) * (x[0] - y[0]) + (x[1] - y[1]) * (x[1] - y[1]));
}

Info info;
int depot_number;
int customer_number;
int vehicle_number;
int capacity;
vector<int> TreeS;
int find(int x) {
    if (TreeS[x] != x) TreeS[x] = find(TreeS[x]);
    return TreeS[x];
}

int demand_all() {
    int demand = 0;
    for (int i = 0; i < info.demands.size(); i++) {
        demand += info.demands[i];
    }
    return demand;
}
int demand = demand_all();
// prevent the change of demand
vector<int> demand_copy() {
    vector<int> demand_cp;
    demand_cp.assign(info.demands.begin(), info.demands.end());
    return demand_cp;
}
vector<int> demand_cp = demand_copy();

vector<CITIES> city() {
    vector<CITIES> v;
    int size = info.loc.size();
    CITIES city;
    for (int i = 0; i < size; i++) {
        city.x = info.loc[i][0];
        city.y = info.loc[i][1];
        if (i < info.depots_num) {
            city.d = 0;
        } else {
            city.d = info.demands[i - info.depots_num];
        }
        v.push_back(city);
    }
    return v;
}

// double adjMatrix[vetex_number][vetex_number] = {0};
vector<vector<double>> adjMatrix;

// calculate distance
double distance(CITIES c1, CITIES c2) {
    double distance = 0;
    distance = sqrt((double)((c1.x - c2.x) * (c1.x - c2.x) + (c1.y - c2.y) * (c1.y - c2.y)));
    return distance;
}

// initial adjacent matrix
void Matrix(const vector<CITIES>& cities) {
    for (int i = 0; i < vetex_number; i++) {
        for (int j = 0; j < vetex_number; j++) {
            CITIES c1 = cities[i];
            CITIES c2 = cities[j];
            adjMatrix[i][j] = distance(c1, c2);
        }
    }
}

vector<int> idx;
vector<double> minDepots(int depots_num) {
    vector<double> tmp;
    for (int i = 0; i < vetex_number; i++) {
        double mini = adjMatrix[i][0];
        int index = 0;
        for (int j = 1; j < depots_num; j++) {
            if (adjMatrix[i][j] < mini) {
                mini = adjMatrix[i][j];
                index = j;
            }
        }
        tmp.push_back(mini);
        idx.push_back(index);
    }
    return tmp;
}

void ReadArc(const vector<vector<double>>& adjMat, vector<Arc>& vertexArc) {
    for (unsigned int i = 0; i < vetex_number; i++) {
        for (unsigned int j = i + 1; j < vetex_number; j++) {
            if (adjMat[i][j] != INFINITE) {
                Arc tmp;
                tmp.u = i;
                tmp.v = j;
                tmp.cost = adjMat[i][j];
                vertexArc.push_back(tmp);
            }
        }
    }
}

void ReadArcDepots(const vector<vector<double>>& adjMat_depots, vector<Arc>& vertexArc) {
    int num = info.customer_num + 1;
    for (unsigned int i = 0; i < num; i++) {
        for (unsigned int j = i + 1; j < num; j++) {
            if (adjMat_depots[i][j] != INFINITE) {
                Arc tmp;
                tmp.u = i;
                tmp.v = j;
                tmp.cost = adjMat_depots[i][j];
                vertexArc.push_back(tmp);
            }
        }
    }
}

bool compare(Arc A, Arc B) { return A.cost < B.cost; }

// Judge whether vertexes are in the same tree, if not, combine and return true; else return false.
bool FindTree(VertexData u, VertexData v) {
    unsigned int index_u = find(u);
    unsigned int index_v = find(v);
    if (index_u != index_v) {
        TreeS[index_u] = index_v;
        return true;
    }
    return false;
}

// double adjMat[vetex_number][vetex_number];
vector<vector<double>> adjMat;
void printTree0(Tree t) {
    cout << endl;
    for (int i = 0; i < vetex_number; i++) {
        for (int j = 0; j < vetex_number; j++) {
            cout << setw(3) << t.tree[i][j];
        }
        cout << endl;
    }
}

Tree Kruskal(const vector<vector<double>>& adjMat) {
    vector<Arc> vertexArc;
    Tree vertexTree;
    // initial path infos
    for (unsigned int i = 0; i < vetex_number; i++) {
        for (unsigned int j = 0; j < vetex_number; j++) {
            vertexTree.tree[i][j] = 0;
            vertexTree.cost = 0;
        }
    }
    ReadArc(adjMat, vertexArc);
    sort(vertexArc.begin(), vertexArc.end(), compare);  // ascending order

    for (int i = 0; i < vetex_number; i++) {
        TreeS[i] = i;
    }
    for (unsigned int i = 0; i < vertexArc.size(); i++) {
        VertexData u = vertexArc[i].u;
        VertexData v = vertexArc[i].v;
        if (FindTree(u, v)) {
            cout << u << "---" << v << endl;
            vertexTree.tree[u][v] = 1;
            vertexTree.cost = vertexArc[i].cost;
        }
    }
    return vertexTree;
}

// minimize spanning tree
TreeDepots Kruskal_depots(const vector<vector<double>>& matrixWithDepots) {
    vector<Arc> vertexArc;
    TreeDepots vertexTree;
    vertexTree.init(vetex_number);
    int num = info.customer_num + 1;
    for (unsigned int i = 0; i < num; i++) {
        for (unsigned int j = 0; j < num; j++) {
            vertexTree.tree[i][j] = 0;
            vertexTree.cost = 0;
        }
    }
    ReadArcDepots(matrixWithDepots, vertexArc);
    sort(vertexArc.begin(), vertexArc.end(), compare);
    for (int i = 0; i < num; i++) {
        TreeS[i] = i;
    }
    for (unsigned int i = 0; i < vertexArc.size(); i++) {
        VertexData u = vertexArc[i].u;
        VertexData v = vertexArc[i].v;
        if (FindTree(u, v)) {
            vertexTree.tree[u][v] = 1;
            vertexTree.cost = vertexArc[i].cost;
        }
    }
    return vertexTree;
}

// double matrixWithDepots[][vetex_number - 1] = {0};
vector<vector<double>> matrixWithDepots;
// double matrixWithMultiDepots[vetex_number][vetex_number] = {0};
vector<vector<double>> matrixWithMultiDepots;
void MultiDepotsToMatrix() {
    vector<double> tmp;
    tmp = minDepots(info.depots_num);
    int num = vetex_number - info.depots_num;
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < num; j++) {
            matrixWithMultiDepots[i][j] = adjMatrix[i + info.depots_num][j + info.depots_num];
        }
    }

    for (int i = 0; i < num; i++) {
        matrixWithMultiDepots[i][num] = tmp[i + info.depots_num];
        matrixWithMultiDepots[num][i] = tmp[i + info.depots_num];
    }
}

// combine two depots
void DepotsToMatrix(int c1, int c2) {
    vector<double> tmp;
    tmp = minDepots(info.depots_num);
    for (int i = 0; i < vetex_number; i++) {
        for (int j = 0; j < vetex_number; j++) {
            if (i < c1 && j < c1) {
                matrixWithDepots[i][j] = adjMatrix[i][j];
            }
            if (i < c1 && c1 < j && j < c2) {
                matrixWithDepots[i][j - 1] = adjMatrix[i][j];
            }
            if (i < c1 && c2 < j) {
                matrixWithDepots[i][j - 2] = adjMatrix[i][j];
            }
            if (c1 < i && i < c2 && j < c1) {
                matrixWithDepots[i - 1][j] = adjMatrix[i][j];
            }
            if (c1 < i && i < c2 && c1 < j && j < c2) {
                matrixWithDepots[i - 1][j - 1] = adjMatrix[i][j];
            }
            if (c1 < i && i < c2 && c2 < j) {
                matrixWithDepots[i - 1][j - 2] = adjMatrix[i][j];
            }
            if (c2 < i && j < c1) {
                matrixWithDepots[i - 2][j] = adjMatrix[i][j];
            }
            if (c2 < i && c1 < j && j < c2) {
                matrixWithDepots[i - 2][j - 1] = adjMatrix[i][j];
            }
            if (c2 < i && c2 < j) {
                matrixWithDepots[i - 2][j - 2] = adjMatrix[i][j];
            }
        }
    }

    int c = 0;
    for (vector<double>::iterator it = tmp.begin(); it != tmp.end(); it++) {
        if (c == c1) {
            it = tmp.erase(it);
        } else if (c == c2) {
            it = tmp.erase(it - 1);
        }
        c = c + 1;
    }
    for (int i = 0; i < vetex_number - 2; i++) {
        matrixWithDepots[i][vetex_number - 2] = tmp[i];
        matrixWithDepots[vetex_number - 2][i] = tmp[i];
    }
}

void printTreeDepots(TreeDepots t) {
    for (int i = 0; i < info.customer_num + 1; i++) {
        for (int j = 0; j < info.customer_num + 1; j++) {
            if (t.tree[i][j] == 1) {
                cout << i << " to " << j << '\n';
            }
        }
    }
}

// Split the tree to forest
Tree TreetoForest(const TreeDepots& d) {
    Tree vertexForest;
    vertexForest.init(vetex_number);
    for (unsigned int i = 0; i < vetex_number; i++) {
        for (unsigned int j = 0; j < vetex_number; j++) {
            vertexForest.tree[i][j] = 0;
            vertexForest.cost = 0;
        }
    }
    vector<Arc> arc;
    int c = 0;
    Arc tmp;
    int n = customer_number;
    for (unsigned int i = 0; i < n; i++) {
        if (d.tree[i][n] == 1) {
            tmp.u = i + info.depots_num;
            tmp.v = idx[i + info.depots_num];
            tmp.cost = d.cost;
            arc.push_back(tmp);
        }
    }
    for (unsigned int i = 0; i < arc.size(); i++) {
        VertexData u = arc[i].u;
        VertexData v = arc[i].v;
        if (u < v) {
            vertexForest.tree[u][v] = 1;
            vertexForest.cost = arc[i].cost;
        } else {
            vertexForest.tree[v][u] = 1;
            vertexForest.cost = arc[i].cost;
        }
    }
    for (unsigned int i = 0; i < info.customer_num; i++) {
        for (unsigned int j = i; j < info.customer_num; j++) {
            vertexForest.tree[i + info.depots_num][j + info.depots_num] = d.tree[i][j];
            vertexForest.cost = d.cost;
        }
    }
    return vertexForest;
}

// Minimize spanning tree
Tree Spanning_forest(const vector<CITIES>& vertices) {
    Matrix(vertices);
    MultiDepotsToMatrix();
    TreeDepots spanning_tree;
    spanning_tree = Kruskal_depots(matrixWithMultiDepots);
    // printTreeDepots(spanning_tree);
    Tree spanning_forest;
    spanning_forest = TreetoForest(spanning_tree);
    // printTreeDepots(spanning_forest);
    // for(int i = 0; i < vetex_number; ++ i){
    //     for(int j = i + 1; j < vetex_number; ++ j){
    //         if(spanning_forest.tree[i][j] == 1){
    //             cout << i << " to " << j << '\n';
    //         }
    //     }
    // }
    return spanning_forest;
}

// Function of finding edge
vector<vector<Arc>> Subedge(const Tree& tree) {
    vector<vector<Arc>> E;
    vector<Arc> E_1;
    vector<Arc> E_2;
    Arc tmp;
    int idx1 = 0;
    for (int i = 0; i < vetex_number; i++) {
        for (int j = i + 1; j < vetex_number; j++) {
            tmp.u = i;
            tmp.v = j;
            tmp.cost = tree.cost;
            tmp.arc_idx = idx1;
            if (tree.tree[i][j] == 1) {
                E_1.push_back(tmp);
            } else {
                E_2.push_back(tmp);
            }
            idx1 = idx1 + 1;
        }
    }
    E.push_back(E_1);
    E.push_back(E_2);
    return E;
}
vector<vector<int>> idx_combine;
// permulation and combination

void generateCombinations(int n, int k, int start_idx, vector<int>& current_combo, vector<vector<int>>& result,
                          int limit) {
    if (current_combo.size() == k) {
        result.push_back(current_combo);
        return;
    }

    if (limit != 0 && result.size() >= limit) {
        return;
    }
    for (int i = start_idx; i <= n - (k - current_combo.size()); ++i) {
        current_combo.push_back(i);
        generateCombinations(n, k, i + 1, current_combo, result, limit);

        current_combo.pop_back();

        if (limit != 0 && result.size() >= limit) {
            return;
        }
    }
}
vector<vector<int>> combine(int size, int num, int limit) {
    idx_combine.clear();

    if (num < 1 || size < num) {
        return idx_combine;
    }

    vector<int> current_combo;
    current_combo.reserve(num);
    if (limit > 0) idx_combine.reserve(limit);

    generateCombinations(size, num, 0, current_combo, idx_combine, limit);

    return idx_combine;
}

vector<vector<Arc>> Add(const vector<vector<Arc>>& edges) {
    vector<vector<Arc>> edges_transfer;
    edges_transfer.push_back(edges[0]);

    for (int k = depot_number - 1; k > 0; k--) {
        vector<vector<int>> idx_0 = combine(edges[0].size(), k, transfer_idx_limit);
        vector<vector<int>> idx_1 = combine(edges[1].size(), k, transfer_idx_limit);

        int start_size = edges_transfer.size();

        for (int i = 0; i < idx_0.size(); i++) {
            bool stop_current_k = false;

            for (int j = 0; j < idx_1.size(); j++) {
                vector<Arc> e = edges[0];

                for (int m = 0; m < k; m++) {
                    int target_pos = idx_0[i][m];
                    int source_pos = idx_1[j][m];

                    e[target_pos] = edges[1][source_pos];
                }

                edges_transfer.push_back(e);

                if (k == 1) {
                    if (edges_transfer.size() >= transfer_limit_all) {
                        stop_current_k = true;
                        break;
                    }
                } else {
                    if ((edges_transfer.size() - start_size) >= transfer_limit) {
                        stop_current_k = true;
                        break;
                    }
                }
            }
            if (stop_current_k) break;
        }
    }
    return edges_transfer;
}

// Verify whether tree is CSF

bool VerifyCSF(const vector<Arc>& arc, int k) {
    for (int i = 0; i < vetex_number; ++i) {
        TreeS[i] = i;
    }

    for (unsigned int i = 0; i < arc.size(); i++) {
        VertexData u = arc[i].u;
        VertexData v = arc[i].v;
        int index_u = find(u);
        int index_v = find(v);

        if (index_u != index_v) {
            if (index_u > index_v) {
                swap(index_u, index_v);
            }
            TreeS[index_v] = index_u;
        } else {
            return false;
        }
    }
    int n = 0;
    unordered_map<int, int> mp;
    for (int i = 0; i < vetex_number; i++) {
        int t = find(i);
        if (TreeS[i] == i) {
            n++;
        }
        mp[t]++;
    }

    for (auto [_, y] : mp) {
        if (y == 1 && _ >= depot_number) {
            return false;
        }
    }
    return true;
}

// Verify whether tree is forest
bool VerifyForest(const vector<Arc>& arc) {
    for (int i = 0; i < vetex_number; ++i) {
        TreeS[i] = i;
    }

    for (unsigned int i = 0; i < arc.size(); i++) {
        int u = arc[i].u;
        int v = arc[i].v;
        int index_u = find(u);
        int index_v = find(v);
        if (index_u != index_v) {
            if (index_u > index_v) {
                swap(index_u, index_v);
            }
            TreeS[index_v] = index_u;
        } else {
            return false;
        }
    }
    int n = 0;
    unordered_map<int, int> mp;
    for (int i = 0; i < vetex_number; i++) {
        int t = find(i);
        mp[t]++;
    }

    for (auto [_, y] : mp) {
        if (y == 1 && _ >= depot_number) {
            return false;
        }
    }
    return true;
}

double compute_cost(const vector<Arc>& arc) {
    double cost = 0;
    double m;
    for (int i = 0; i < arc.size(); i++) {
        m = adjMatrix[arc[i].u][arc[i].v];
        cost = cost + m;
    }
    return cost;
}

vector<vector<VertexData>> edgeToTree(const vector<Arc>& arc) {
    for (unsigned int i = 0; i < vetex_number; i++) {
        TreeS[i] = i;
    }
    for (unsigned int i = 0; i < arc.size(); i++) {
        VertexData u = arc[i].u;
        VertexData v = arc[i].v;
        int index_u = find(u);
        int index_v = find(v);
        if (index_u != index_v) {
            TreeS[index_u] = index_v;
        }
    }
    unordered_map<int, int> mp;
    vector<vector<VertexData>> vtree;
    for (int i = 0; i < vetex_number; i++) {
        int t = find(i);
        mp[t]++;
    }
    for (auto [x, y] : mp) {
        if (y > 1) {
            vector<VertexData> tmp;
            for (int i = 0; i < vetex_number; i++) {
                int t = find(i);
                if (t == x) {
                    tmp.push_back(i);
                }
            }
            vtree.push_back(tmp);
        }
    }
    return vtree;
}

// Add edges between two trees
vector<Arc> addArc(const vector<vector<VertexData>>& tree) {
    vector<Arc> arclist;
    Arc tmp;
    int treeNum = tree.size();
    for (int i = 0; i < treeNum - 1; i++) {
        for (int j = i + 1; j < treeNum; j++) {
            for (int m = 0; m < tree[i].size(); m++) {
                for (int n = 0; n < tree[j].size(); n++) {
                    tmp.u = tree[i][m];
                    tmp.v = tree[j][n];
                    tmp.cost = adjMatrix[tmp.u][tmp.v];
                    arclist.push_back(tmp);
                }
            }
        }
    }
    return arclist;
}

// Add edges between multiple trees and select edge with minimum cost
vector<Arc> addArcMultiDepots(const vector<vector<VertexData>>& tree) {
    vector<Arc> miniarclist;

    int treeNum = tree.size();
    for (int i = 0; i < treeNum - 1; i++) {
        int j = i + 1;
        vector<Arc> arclist;
        Arc mini;
        double minCost = 1e9;
        for (int m = 0; m < tree[i].size(); m++) {
            for (int n = 0; n < tree[j].size(); n++) {
                Arc tmp;
                tmp.u = tree[i][m];
                tmp.v = tree[j][n];
                tmp.cost = adjMatrix[tmp.u][tmp.v];
                if (minCost > tmp.cost) {
                    minCost = tmp.cost;
                    mini = tmp;
                }
            }
        }
        miniarclist.push_back(mini);
    }
    return miniarclist;
}

bool feasibility(const vector<vector<VertexData>>& tree, const vector<CITIES>& vertices) {
    int vehicle_demand = 0;
    for (int i = 0; i < tree.size(); i++) {
        int demands = 0;
        for (int j = 0; j < tree[i].size(); j++) {
            demands = demands + vertices[tree[i][j]].d;
        }
        int p = ceil(double(demands) / capacity);
        vehicle_demand += p;
    }
    if (vehicle_demand > vehicle_number) {
        return false;
    } else
        return true;
}

// Verify the degree
vector<int> Odd(const vector<Arc>& forest) {
    vector<int> degree(vetex_number, 0);
    for (int i = 0; i < forest.size(); i++) {
        int u = forest[i].u;
        int v = forest[i].v;
        degree[u]++;
        degree[v]++;
    }
    vector<int> oddvertex;
    for (int j = 0; j < vetex_number; j++) {
        if (degree[j] % 2 == 1) {
            oddvertex.push_back(j);
        }
    }
    return oddvertex;
}

#define inf 0x3f3f3f3f
int nx, ny;
int link[M], lx[M], ly[M], slack[M];
int visx[M], visy[M];
double w[M][M];

int DFS(int x) {
    visx[x] = 1;
    for (int y = 1; y <= ny; y++) {
        if (visy[y]) continue;
        int t = lx[x] + ly[y] - w[x][y];
        if (t == 0)  //
        {
            visy[y] = 1;
            if (link[y] == -1 || DFS(link[y])) {
                link[y] = x;
                return 1;
            }
        } else if (slack[y] > t)
            slack[y] = t;
    }
    return 0;
}

vector<int> KM() {
    int i, j;
    memset(link, -1, sizeof(link));
    memset(ly, 0, sizeof(ly));
    for (i = 1; i <= nx; i++)
        for (j = 1, lx[i] = -inf; j <= ny; j++)
            if (w[i][j] > lx[i]) lx[i] = w[i][j];
    for (int x = 1; x <= nx; x++) {
        for (i = 1; i <= ny; i++) slack[i] = inf;
        while (1) {
            memset(visx, 0, sizeof(visx));
            memset(visy, 0, sizeof(visy));
            if (DFS(x)) break;
            int d = inf;
            for (i = 1; i <= ny; i++)
                if (!visy[i] && d > slack[i]) d = slack[i];
            for (i = 1; i <= nx; i++)
                if (visx[i]) lx[i] -= d;
            for (i = 1; i <= ny; i++)
                if (visy[i])
                    ly[i] += d;
                else
                    slack[i] -= d;
        }
    }
    double res = 0;
    vector<int> v;
    for (i = 1; i <= ny; i++)
        if (link[i] > -1) {
            res += w[link[i]][i];
            v.push_back(link[i]);
        }
    return v;
}

// Minimum matching
vector<int> MatchingKM(const vector<int>& Odds) {
    int num = Odds.size();
    // double adjM[num][num];
    vector<vector<double>> adjM(num, vector<double>(num, 0));
    for (int a = 0; a < num; a++) {
        for (int b = 0; b < num; b++) {
            int p;
            int q;
            p = Odds[a];
            q = Odds[b];
            adjM[a][b] = adjMatrix[p][q];
        }
    }
    for (int i = 1; i <= num; i++) {
        for (int j = 1; j <= num; j++) {
            if (i == j) {
                w[i][j] = -inf;
            } else {
                w[i][j] = 0 - adjM[i - 1][j - 1];
            }
        }
    }
    nx = ny = num;
    vector<int> link;
    link = KM();
    return link;
}

std::vector<Arc> MinimumCostPerfectMatching(const std::vector<int>& Odds) {
    int u, v;
    int sz = Odds.size();
    int edge_num = sz * (sz - 1) / 2;
    PerfectMatching* pm = new PerfectMatching(sz, edge_num);
    
    for (int i = 0; i < sz; i++) {
        for (int j = i + 1; j < sz; j++) {
            int u = Odds[i];
            int v = Odds[j];
            int c = ceil(adjMatrix[u][v]*10);
            pm->AddEdge(i, j, c);
        }
    }
    std::vector<Arc> result;  
    // for (int i = 1; i <= sz; ++i) {
    //     if (i < blossom.mat[i]) {
    //         Arc arc;
    //         arc.u = Odds[i - 1];
    //         arc.v = Odds[blossom.mat[i] - 1];
    //         arc.cost = adjMatrix[arc.u][arc.v];
    //         result.push_back(arc);
    //     }
    // }

    pm->Solve();
    std::unordered_set<int> matched;
    for (int i = 0; i < sz; ++i) {
        if (!matched.count(Odds[i])) {
            Arc edge;
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

vector<vector<int>> Matching_vd(const vector<vector<int>>& s) {
    int num = vehicle_number;
    vector<vector<int>> d_s;
    vector<int> xy;
    vector<int> min_index;
    for (int m = 0; m < depot_number; m++) {
        vector<int> d_s_min;
        for (int i = 0; i < s.size(); i++) {
            int min = adjMatrix[m][s[i][0]];
            int index = 0;
            int tmp = 0;
            for (int j = 1; j < s[i].size(); j++) {
                tmp = adjMatrix[m][s[i][j]];
                if (min > tmp) {
                    min = tmp;
                    index = j;
                }
            }
            min_index.push_back(index);
            d_s_min.push_back(min);
        }
        d_s.push_back(d_s_min);
    }
    vector<int> ind_vd;

    for (int i = 1; i <= num; i++) {
        int index = 0, v = 0;
        for (int n = 0; n < info.vehicles.size(); n++) {
            v = v + info.vehicles[n];
            if (i <= v) {
                index = n;
                break;
            }
        }
        ind_vd.push_back(index);
        for (int j = 1; j <= s.size(); j++) {
            if (index < ind_vd.size() && j - 1 < d_s[index].size()) {
                w[i][j] = 0 - d_s[index][j - 1];
            }
        }
        for (int m = vehicle_number; m > s.size(); m--) {
            w[i][m] = 0;
        }
    }
    nx = ny = num;
    vector<int> link;
    link = KM();
    vector<vector<int>> all_path;
    for (int i = 0; i < s.size(); i++) {
        vector<int> path;
        if (find(s[i].begin(), s[i].end(), ind_vd[link[i] - 1]) != s[i].end()) {
            auto it = find(s[i].begin(), s[i].end(), ind_vd[link[i] - 1]);
            path.assign(it, s[i].end());
            path.insert(path.end(), s[i].begin(), it);
        } else {
            path.push_back(ind_vd[link[i] - 1]);
            path.push_back(s[i][min_index[i]]);
            for (int j = min_index[i] - 1; j >= 0; j--) {
                path.push_back(s[i][j]);
            }
            for (int j = min_index[i] + 1; j < s[i].size(); j++) {
                path.push_back(s[i][j]);
            }
        }
        path.push_back(ind_vd[link[i] - 1]);
        all_path.push_back(path);
    }
    return all_path;
}

bool checkMatching(const vector<Arc>& arcs) {
    vector<int> degree(vetex_number, 0);
    for (auto it = arcs.begin(); it != arcs.end(); it++) {
        degree[it->u]++;
        degree[it->v]++;
    }
    for (int i = 0; i < vetex_number; i++) {
        if (degree[i] % 2 != 0) {
            return false;
        }
    }
    return true;
}

vector<vector<int>> shortcut(vector<Arc> arcs) {
    vector<vector<int>> path_all;
    vector<int> degree(vetex_number, 0);
    while (arcs.size() != 0) {
        for (int i = 0; i < vetex_number; ++i) {
            degree[i] = 0;
        }
        // The degree of each node is counted
        for (vector<Arc>::iterator it = arcs.begin(); it != arcs.end(); it++) {
            degree[it->u]++;
            degree[it->v]++;
        }
        vector<int> path;
        // The first edge is added directly to path
        path.push_back(arcs[0].u);
        path.push_back(arcs[0].v);
        int p = arcs[0].v;
        degree[arcs[0].u]--;
        degree[arcs[0].v]--;
        arcs.erase(arcs.begin());
        // Find edge
        while (degree[p] != 0) {
            vector<Arc> arc_cp;
            int p_add_n = 0;
            for (vector<Arc>::iterator it = arcs.begin(); it != arcs.end(); it++) {
                if (p == it->u) {
                    p = it->v;
                    path.push_back(p);
                    degree[it->u]--;
                    degree[it->v]--;
                    p_add_n++;
                } else if (p == it->v) {
                    p = it->u;

                    path.push_back(p);
                    p_add_n++;
                    degree[it->u]--;
                    degree[it->v]--;
                } else {
                    arc_cp.push_back(*it);
                }
            }
            arcs.clear();
            arcs.assign(arc_cp.begin(), arc_cp.end());
        }
        path_all.push_back(path);
    }
    return path_all;
}

vector<int> path_cut(const vector<int>& path) {
    vector<int> path_after;
    path_after.push_back(path[0]);
    for (int i = 1; i < path.size(); i++) {
        if (find(path_after.begin(), path_after.end(), path[i]) == path_after.end()) {
            path_after.push_back(path[i]);
        }
    }
    return path_after;
}

vector<int> findPath(vector<vector<int>> path_all) {
    vector<int> path = path_cut(path_all[0]);
    for (int i = 0; i < path_all.size() - 1; i++) {
        int start = path_all[i + 1][0];
        if (find(path.begin(), path.end(), start) != path.end()) {
            vector<int> path_cp;
            path.insert(find(path.begin(), path.end(), start) + 1, path_all[i + 1].begin() + 1, path_all[i + 1].end());
            path = path_cut(path);
        } else {
            vector<int> path_cp;
            for (int j = 1; j < path_all[i + 1].size(); j++) {
                start = path_all[i + 1][j];
                if (find(path.begin(), path.end(), start) != path.end()) {
                    vector<int>::iterator it = find(path_all[i + 1].begin(), path_all[i + 1].end(), start);
                    path_cp.insert(path_cp.begin(), it, path_all[i + 1].end());
                    path_cp.insert(path_cp.end(), path_all[i + 1].begin(), it);
                    path.insert(find(path.begin(), path.end(), start) + 1, path_cp.begin() + 1, path_cp.end());
                    path = path_cut(path);
                    break;
                } else if (j == path_all[i + 1].size() - 1 && find(path.begin(), path.end(), start) == path.end()) {
                    path.insert(path.end(), path_all[i + 1].begin(), path_all[i + 1].end());
                    path = path_cut(path);
                    break;
                }
            }
        }
    }
    if (path.size() == vetex_number) {
        return path;
    } else {
        return vector<int>();
    }
}

pair<vector<vector<int>>, vector<map<int, int>>> cycle_split(const vector<int>& path) {
    vector<vector<int>> S;
    vector<map<int, int>> demand_list;
    // Calculate n_c
    int demands = 0;
    for (int j = 0; j < info.demands.size(); j++) {
        demands = demands + info.demands[j];
    }
    int n_c = ceil((double)demands / capacity);
    // Copy to demand_cp (demands need to be changed later)
    // int demand_cp[vetex_number] = {0};
    vector<int> demand_cp(vetex_number, 0);
    for (int a = 0; a < vetex_number; a++) {
        if (a >= depot_number) {
            demand_cp[a] = info.demands[a - depot_number];
        } else {
            demand_cp[a] = 0;
        }
    }
    int p = 0;
    for (int m = 0; m < n_c; m++) {
        int q = 0;
        vector<int> s_p;
        int d = 0;
        map<int, int> x;
        for (int n = p; n < path.size(); n++) {
            if (path[n] >= depot_number) {
                q = q + demand_cp[path[n]];
            }
            s_p.push_back(path[n]);
            p = n;
            if (path[n] >= depot_number) {
                if (q >= capacity) {
                    d += demand_cp[path[n]] - (q - capacity);
                    x[path[n]] = demand_cp[path[n]] - (q - capacity);
                    demand_cp[path[n]] = q - capacity;
                    break;
                } else {
                    d += demand_cp[path[n]];
                    x[path[n]] = demand_cp[path[n]];
                }
            }
        }
        demand_list.push_back(x);
        S.push_back(s_p);
    }
    return pair<vector<vector<int>>, vector<map<int, int>>>(S, demand_list);
}

void printRoute(vector<vector<int>> s) {
    cout << "-------path---------" << endl;
    for (int i = 0; i < s.size(); i++) {
        int depot = s[i][0];
        cout << depot << ":";
        for (int j = 0; j < s[i].size(); j++) {
            cout << setw(8) << s[i][j];
        }
        cout << endl;
    }
}

double routing(const vector<vector<int>>& s) {
    double all_cost = 0;
    for (int i = 0; i < s.size(); i++) {
        int p, q;
        double cost = 0;
        for (int j = 0; j < s[i].size() - 1; j++) {
            p = s[i][j];
            q = s[i][j + 1];
            cost = cost + adjMatrix[p][q];
        }
        all_cost = all_cost + cost;
    }
    return all_cost;
}

vector<vector<Arc>> forest_add_all(const vector<vector<Arc>>& forest_all, const vector<CITIES>& vertices) {
    vector<vector<Arc>> forest_add;
    int limit_num = add_limit;
    int forestCount = 0;
    int feasibleCount = 0;
    int add_num_5 = 0;
    int add_num_4 = 0;
    int add_num_3 = 0;
    int add_num_2 = 0;
    int add_num_1 = 0;
    for (int i = 0; i < forest_all.size(); i++) {
        vector<vector<VertexData>> tree = edgeToTree(forest_all[i]);
        vector<Arc> arc_list;
        if (depot_number == 2) {
            arc_list = addArc(tree);
        } else {
            arc_list = addArcMultiDepots(tree);
        }
        int k = depot_number - 1;
        while (k > 0) {
            if (forest_add.size() >= limit_num) break;

            vector<vector<int>> idx = combine(arc_list.size(), k, transfer_idx_limit);

            for (int j = 0; j < idx.size(); j++) {
                vector<Arc> forest = forest_all[i];
                for (int m = 0; m < k; m++) {
                    forest.push_back(arc_list[idx[j][m]]);
                }
                if (VerifyForest(forest)) {
                    forestCount++;
                    vector<vector<VertexData>> isforest = edgeToTree(forest);

                    if (feasibility(isforest, vertices)) {
                        forest_add.push_back(forest);
                        feasibleCount++;
                    }
                }
                if (forest_add.size() >= limit_num) {
                    break;
                }
            }
            if (k == 1) {
                if (forest_add.size() < limit_num) {
                    if (VerifyForest(forest_all[i])) {
                        forestCount++;
                        vector<vector<VertexData>> isforest = edgeToTree(forest_all[i]);
                        if (feasibility(isforest, vertices)) {
                            forest_add.push_back(forest_all[i]);
                        }
                    }
                }
            }

            k--;
        }
    }
    return forest_add;
}

vector<vector<int>> transfer_route(vector<vector<int>> path, vector<int> idx) {
    for (int i = 0; i < path.size(); i++) {
        for (int j = 0; j < path[i].size(); j++) {
            if (path[i][j] == idx[0]) {
                path[i][j] = idx[1];
            } else if (path[i][j] == idx[1]) {
                path[i][j] = idx[0];
            }
        }
    }
    return path;
}

vector<vector<int>> transfer_cmb() {
    vector<vector<int>> idx_combine;
    int b = depot_number;
    int e = vetex_number;
    for (int i = b; i < e - 1; i++) {
        for (int j = i + 1; j < e; j++) {
            vector<int> idx;
            idx.push_back(i);
            idx.push_back(j);
            idx_combine.push_back(idx);
        }
    }
    return idx_combine;
}

double calculateCost(vector<vector<int>> s) {
    double all_cost = 0;
    for (int i = 0; i < s.size(); i++) {
        int p, q;
        double cost = 0;
        for (int j = 0; j < s[i].size() - 1; j++) {
            p = s[i][j];
            q = s[i][j + 1];
            cost = cost + adjMatrix[p][q];
        }
        all_cost = all_cost + cost;
    }
    return all_cost;
}

void BFS(int s, int t, vector<vector<int>>& C, vector<vector<int>>& F, vector<int>& L) {
    int n = L.size();
    vector<int> D(n, INT_MAX);
    vector<bool> visited(n);
    L[s] = 0;
    while (1) {
        int i = 0;
        while (i < n && (visited[i] || L[i] == INT_MAX)) {
            i++;
        }
        if (i == n) {
            return;
        }
        visited[i] = true;
        for (int j = 0; j < n; j++) {
            if (L[j] == INT_MAX) {
                if (F[i][j] < C[i][j]) {
                    D[j] = min(D[i], C[i][j] - F[i][j]);
                    L[j] = i;
                } else if (F[j][i] > 0) {
                    D[j] = min(D[i], F[j][i]);
                    L[j] = -i;
                }
                if (L[t] != INT_MAX) {
                    do {
                        if (L[j] >= 0) {
                            F[L[j]][j] += D[t];
                            j = L[j];
                        } else {
                            F[j][-L[j]] -= D[t];
                            j = -L[j];
                        }
                    } while (j != s);
                    return;
                }
            }
        }
    }
}

bool is_feasible(vector<vector<int>> route) {
    int n, m, s, t;
    n = vetex_number - depot_number + 2;
    s = 0;
    t = n - 1;
    m = depot_number - 1;
    vector<vector<int>> C(n, vector<int>(n));  // Adjacency matrix with the value of capacity
    // Start point is 0, n
    for (int i = 0; i < route.size(); i++) {
        C[s][route[i][1] - m] += capacity;
        for (int j = 2; j < route[i].size() - 1; j++) {
            C[route[i][1] - m][route[i][j] - m] += capacity;
        }
    }
    for (int i = 1; i < vetex_number - m; i++) {
        C[i][t] = demand_cp[i - 1];
    }
    vector<vector<int>> F(n, vector<int>(n));
    vector<int> L(n, INT_MAX);
    do {
        fill(L.begin(), L.end(), INT_MAX);
        BFS(s, t, C, F, L);
    } while (L[t] != INT_MAX);
    int totalFlow = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (C[i][j] > 0) {
                if (i == 0) {
                    totalFlow += F[i][j];
                }
            }
        }
    }
    if (totalFlow == demand) {
        return true;
    } else
        return false;
}
set<vector<Arc>> visited;

int main(int argc, char* argv[]) {
    string alg, strInputFile, strOutputFile;

    if (argc < 3) {
        throw std::runtime_error("Not enough parameters have been passed. \n");
    } else {
        strInputFile = argv[1];
        strOutputFile = argv[2];
        alg = argv[3];
    }

    // strInputFile = "../data/SD_set/k=4/SD2.txt";
    // alg = "GREEDY";
    // strOutputFile = "../output_SD2.txt";

    if (strInputFile.find("P_set") != string::npos) {
        data_type = 1;
        path_1_depots = strInputFile;
    } else if (strInputFile.find("SD_set") != string::npos) {
        data_type = 0;
        path_0_depots = strInputFile;
    } else {
        data_type = 2;
        path_2_depots = strInputFile;
    }

    vector<vector<int>> min_route;
    vector<map<int, int>> best_demand;

    clock_t start, mid, end;
    start = clock();
    string result_path = strOutputFile;
    std::ofstream file(result_path);
    if (!file) {
        throw std::runtime_error("open file error\n");
    }

    std::streambuf* original_cout_buffer = std::cout.rdbuf();

    std::cout.rdbuf(file.rdbuf());

    try {
        cout << result_path << endl;
        info = readTxt();
        depot_number = info.depots_num;
        customer_number = info.customer_num;
        vehicle_number = info.vehicle_num;
        capacity = info.capacity;
        vetex_number = customer_number + depot_number;

        cout << "vehicle number : " << vehicle_number << endl;

        TreeS.resize(vetex_number * 2);
        for (int i = 0; i < vetex_number; ++i) {
            vector<double> tmp(vetex_number, 0);
            adjMat.push_back(tmp);
            adjMatrix.push_back(tmp);
            matrixWithDepots.push_back(tmp);
            matrixWithMultiDepots.push_back(tmp);
        }

        // w.resize(info.vehicle_num + 1);
        // for(int i = 0; i < w.size(); i++) {
        //     w[i].resize(info.vehicle_num + 1, 0);
        // }
        vector<CITIES> vertices;
        vertices = city();
        Tree spanning_forest = Spanning_forest(vertices);
        vector<vector<Arc>> e = Subedge(spanning_forest);
        cout << e[0].size() << ' ' << e[1].size() << '\n';
        double min_forest_cost = compute_cost(e[0]);
        // Exchange edge
        int cnt = 0;
        int rd = 0;
        int sz1 = e[0].size();
        int sz2 = e[1].size();

        double min_cost = inf;
        vector<vector<int>> min_route;
        vector<map<int, int>> best_demand;

        bool flag = false;
        while (true) {
            mid = clock();
            double tm = (double)(mid - start) / CLOCKS_PER_SEC;
            if (tm >= 3600) break;

            if (rd) {
                int cnt1 = 0;
                auto ed = e[0].back();
                for (int i = sz1 - 1; i >= 1; --i) {
                    e[0][i] = e[0][i - 1];
                }
                e[0][0] = ed;

                ed = e[1].back();
                for (int i = sz2 - 1; i >= 1; --i) {
                    e[1][i] = e[1][i - 1];
                }
                e[1][0] = ed;
            }
            rd++;

            vector<vector<Arc>> t = Add(e);
            // cout << t.size() << '\n';
            int n = depot_number;
            vector<vector<Arc>> forest_all;
            for (int i = 0; i < t.size(); i++) {
                bool isCSF = VerifyCSF(t[i], n);
                if (isCSF) {
                    auto temp = t[i];
                    sort(temp.begin(), temp.end());
                    if (visited.count(temp)) {
                        continue;
                    } else {
                        visited.insert(temp);
                        forest_all.push_back(t[i]);
                    }
                    // cout << "1111111111\n";
                }
            }
            // cout << forest_all.size() << '\n';
            vector<vector<Arc>> forest_add = forest_add_all(forest_all, vertices);
            // cout << forest_add.size() << '\n';

            for (int m = 0; m < forest_add.size(); m++) {
                vector<int> Odds = Odd(forest_add[m]);
                vector<Arc> match_arc;
                match_arc = MinimumCostPerfectMatching(Odds);
                vector<Arc> combine;
                combine.insert(combine.end(), forest_add[m].begin(), forest_add[m].end());
                combine.insert(combine.end(), match_arc.begin(), match_arc.end());

                if (checkMatching(combine)) {
                    vector<vector<int>> path_all;
                    path_all = shortcut(combine);
                    vector<int> path = findPath(path_all);
                    if (path.size() == 0) continue;
                    //        vector<vector<int>> s;
                    pair<vector<vector<int>>, vector<map<int, int>>> section = cycle_split(path);
                    vector<vector<int>> s = section.first;
                    vector<map<int, int>> s_demand = section.second;
                    vector<vector<int>> depots = Matching_vd(s);
                    vector<vector<int>> depots_cp;
                    for (int i = 0; i < depots.size(); i++) {
                        vector<int> d;
                        d.push_back(depots[i][0]);
                        for (int j = 1; j < depots[i].size() - 1; j++) {
                            if (depots[i][j] >= depot_number) {
                                d.push_back(depots[i][j]);
                            }
                        }
                        d.push_back(depots[i][depots[i].size() - 1]);
                        depots_cp.push_back(d);
                    }

                    double cost = routing(depots_cp);

                    if (min_cost > cost) {
                        min_cost = cost;
                        min_route = depots_cp;
                        best_demand = s_demand;
                    }
                    cnt++;
                    mid = clock();
                    tm = (double)(mid - start) / CLOCKS_PER_SEC;
                    if (tm >= 3600) {
                        flag = true;
                        break;
                    }
                }
            }
            if (flag) {
                break;
            }
        }
        cout << "valid solutions : " << cnt << endl;
        std::cout << std::fixed << std::setprecision(10) << std::noshowpoint;
        cout << endl;
        cout << "======================================" << endl;
        cout << "The minimum cost is（without opt-2）:" << min_cost << endl;
        cout << "The minimum cost is:" << min_cost << endl;
        cout << "--------------------" << endl;
        mid = clock();
        cout << "The run time （without opt-2）is: " << double(mid - start) / CLOCKS_PER_SEC << " (s) " << endl;
        cout << "The run time is: " << double(mid - start) / CLOCKS_PER_SEC << " (s) " << endl;
        cout << "The optimal route and corresponding demands serviced:" << endl;
        // printRoute(min_route);
        double cost = 0;
        vector<int> tmp_demand(vetex_number + 1, 0);
        cout << min_route.size() << endl;
        for (int i = 0; i < min_route.size(); i++) {
            int d = 0;
            double tmp_cost = 0;
            for (int j = 0; j < min_route[i].size(); j++) {
                if (j == 0) {
                    cout << min_route[i][j];
                } else if (j == (min_route[i].size() - 1)) {
                    cout << setw(5) << min_route[i][j];
                    cout << "    total service:" << d;
                } else {
                    int x = best_demand[i][min_route[i][j]];
                    tmp_demand[min_route[i][j]] += x;
                    cout << setw(5) << min_route[i][j] << "_" << x;
                    d += x;
                }
                if (j) {
                    tmp_cost += distance(vertices[min_route[i][j - 1]], vertices[min_route[i][j]]);
                }
            }
            cout << endl << setw(10) << tmp_cost << endl;
            cost += tmp_cost;
            cout << endl;
        }
        // cout << "    total demand:";
        for (int i = depot_number; i < vetex_number; i++) {
            int cust = i - depot_number;
            if (info.demands[cust] != tmp_demand[i]) {
                cout << "Demand error at customer " << i << ": expected " << info.demands[cust] << ", got "
                     << tmp_demand[i] << endl;
            }
        }
        cout << endl;
        cout << setw(10) << cost << endl;
    } catch (const std::exception& e) {
        std::cout.rdbuf(original_cout_buffer);
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout.rdbuf(original_cout_buffer);
    return 0;
}
