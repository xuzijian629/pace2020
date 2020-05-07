#pragma once
#include "graph.cpp"

int degeneracy(const Graph& g) {
    unordered_map<int, int> mp;
    for (int v : g.nodes) mp[v];
    int N = 0;
    for (auto& p : mp) p.second = N++;
    vector<vector<int>> adj(N);
    for (int a : g.nodes) {
        for (int b : g.adj.at(a)) {
            adj[mp[a]].push_back(mp[b]);
        }
    }
    vector<bool> visited(N);
    vector<int> deg(N);
    vector<unordered_set<int>> D(N);
    for (int i = 0; i < N; i++) {
        deg[i] = adj[i].size();
        D[deg[i]].insert(i);
    }
    int ret = 0;
    int s = 0;
    for (int _ = 0; _ < N; _++) {
        for (int i = 0; i < N; i++) {
            if (D[i].empty()) continue;
            ret = max(ret, i);
            int v = *D[i].begin();
            D[i].erase(v);
            visited[v] = true;
            for (int w : adj[v]) {
                if (!visited[w]) {
                    assert(D[deg[w]].count(w));
                    D[deg[w]].erase(w);
                    deg[w]--;
                    D[deg[w]].insert(w);
                }
            }
            break;
        }
    }

    return ret;
}

// compute near-optimal tree decomposition
int treewidth_lb(const Graph& g) { return degeneracy(g); }
