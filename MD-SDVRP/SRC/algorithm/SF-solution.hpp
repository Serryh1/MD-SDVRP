
#include <queue>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class steinerForest {
   public:
   steinerForest(int V);
    ~steinerForest(); 
    void addEdge(int v, int w, double wei);
    void deleteEdge(int u, int v);
    class node {
       public:
        node(int i, double w) {
            index = i;
            needed = true;
            weight = w;
        }
        int index;
        bool needed;
        double weight;
    };
    int V;              // No. of vertices, initialy we add all vertices
    vector<node>* adj;  // Pointer to an array containing adjacency lists
    vector<int> Path;
    void dfs(int s);
    vector<bool> visit;
    vector<int> node_values;
    bool BFSCleanSolution(const int& u, const int& v, const int& Q);
    // edge(u,v) can be cut
};

inline steinerForest::steinerForest(int V) {
    this->V = V;
    adj = new vector<node>[V];
}

inline void steinerForest::addEdge(int v, int w, double wei) {
    node vNode(v, wei);
    node WNode(w, wei);
    adj[v].push_back(WNode);  // Add w to v’s list.
    adj[w].push_back(vNode);  // Add v to w’s list.
}
inline steinerForest::~steinerForest() {
    if (adj != nullptr) {
        delete[] adj; 
    }
}
// delete this edge
inline void steinerForest::deleteEdge(int u, int v) {
    int x = -1;
    for (int i = 0; i < adj[u].size(); ++i) {
        if (adj[u][i].index == v) {
            x = i;
            break;
        }
    }
    if (x == -1) return;
    adj[u].erase(adj[u].begin() + x);
    int y = -1;
    for (int i = 0; i < adj[v].size(); ++i) {
        if (adj[v][i].index == u) {
            y = i;
            break;
        }
    }
    if (y == -1) return;
    adj[v].erase(adj[v].begin() + y);
}

inline bool steinerForest::BFSCleanSolution(const int& u, const int& v, const int& Q) {
    queue<int> q;
    unordered_map<int, int> visited;
    q.push(u);
    bool flag = false;
    visited[v] = 1;
    std::unordered_set<int> se;
    int cnt = 0;
    while (!q.empty()) {
        int t = q.front();
        q.pop();
        if (visited[t]) continue;
        visited[t] = true;
        cnt += node_values[t];
        se.insert(t);
        for (const auto& NODE : adj[t]) {
            int to = NODE.index;
            if (visited[to]) continue;
            q.push(to);
        }
    }
    if (cnt % Q > 0) return false;
    return true;
}

inline void steinerForest::dfs(int u) {
    visit[u] = true;
    // cout << u << ' ';
    Path.push_back(u);
    for (int i = 0; i < adj[u].size(); i++) {
        int v = adj[u][i].index;
        if (!visit[v]) {
            dfs(v);
        }
    }
}

// void steinerForest::dfs(int u) {
//     std::stack<int> st;
//     st.push(u);

//     while (!st.empty()) {
//         int v = st.top();
//         if (!adj[v].empty()) {
//             int neighbor = adj[v].back().index;
//             adj[v].pop_back();
//             for (auto it = adj[neighbor].begin(); it != adj[neighbor].end(); ++it) {
//                 if (it->index == v) {
//                     *it = adj[neighbor].back();
//                     adj[neighbor].pop_back();
//                     break;
//                 }
//             }

//             st.push(neighbor);
//         } else {
//             Path.push_back(v);
//             st.pop();
//         }
//     }
//     reverse(Path.begin(), Path.end());
// }