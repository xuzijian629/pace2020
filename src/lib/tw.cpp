#pragma once
#include "graph.cpp"
#include "minimal_separator.cpp"

namespace std {
template <>
struct hash<unordered_set<int>> {
    inline size_t operator()(const unordered_set<int>& s) const { return get_hash(s, 1333); }
};
}  // namespace std

bool all_feasible(const Graph& g, const unordered_set<int>& S, const unordered_set<int>& C,
                  const unordered_set<unordered_set<int>>& min_seps, int k,
                  const unordered_set<unordered_set<int>>& is_feasible_memo);

bool is_feasible(const Graph& g, const unordered_set<int>& C, const unordered_set<unordered_set<int>>& min_seps, int k,
                 const unordered_set<unordered_set<int>>& is_feasible_memo) {
    if (min_seps.count(open_neighbors(g, C)) && close_neighbors(g, C).size() <= k + 1) return true;
    int min_C = get_min(C, {});
    for (auto& D : is_feasible_memo) {
        if (D.count(min_C) &&
            all_feasible(g, join(open_neighbors(g, C), open_neighbors(g, D)), C, min_seps, k, is_feasible_memo))
            return true;
    }
    return all_feasible(g, join(open_neighbors(g, C), unordered_set<int>({min_C})), C, min_seps, k, is_feasible_memo);
}

bool all_feasible(const Graph& g, const unordered_set<int>& S, const unordered_set<int>& C,
                  const unordered_set<unordered_set<int>>& min_seps, int k,
                  const unordered_set<unordered_set<int>>& is_feasible_memo) {
    if (S.size() > k + 1) return false;
    for (auto& D : components(remove(g, S))) {
        if (is_subset(D.nodes, C)) {
            if (open_neighbors(g, D.nodes) == S) {
                if (!is_feasible(g, D.nodes, min_seps, k, is_feasible_memo)) return false;
            } else {
                if (!is_feasible_memo.count(D.nodes)) return false;
            }
        }
    }
    return true;
}

bool msdp(const Graph& g, const unordered_set<unordered_set<int>>& min_seps, int k) {
    vector<unordered_set<int>> inbounds;
    for (auto& s : min_seps) {
        vector<unordered_set<int>> C;
        vector<int> mins;
        int k = 0;
        for (auto& conn : components(remove(g, s))) {
            if (s != open_neighbors(g, conn.nodes)) continue;
            C.push_back(conn.nodes);
            mins.push_back(get_min(conn.nodes, {}));
        }
        int nin = *min_element(mins.begin(), mins.end());
        if (C.size() > 1) {
            for (int i = 0; i < C.size(); i++) {
                if (mins[i] != nin) {
                    inbounds.push_back(C[i]);
                }
            }
        }
    }
    sort(inbounds.begin(), inbounds.end(), [](auto& a, auto& b) { return a.size() < b.size(); });
    unordered_set<unordered_set<int>> is_feasible_memo;
    for (auto& C : inbounds) {
        if (is_feasible(g, C, min_seps, k, is_feasible_memo)) is_feasible_memo.insert(C);
    }
    return is_feasible(g, g.nodes, min_seps, k, is_feasible_memo);
}

bool treewidth_des(const Graph& g, int k) {
    auto seps = list_exact(g, k);
    return msdp(g, unordered_set<unordered_set<int>>(seps.begin(), seps.end()), k);
}

int treewidth_exact(const Graph& g) {
    int k = 0;
    while (1) {
        if (treewidth_des(g, k)) return k;
        k++;
    }
}

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

int treewidth_lb(const Graph& g) { return degeneracy(g); }

// compute near-optimal tree decomposition
int treewidth_ub(const Graph& g) { return treewidth_exact(g); }
