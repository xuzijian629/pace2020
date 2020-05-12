#pragma once
#include "graph.cpp"
#include "minimal_separator.cpp"

bool all_feasible(const Graph& g, const BITSET& S, const BITSET& C, const unordered_set<BITSET>& min_seps, int k,
                  const unordered_set<BITSET>& is_feasible_memo);

bool is_feasible(const Graph& g, const BITSET& C, const unordered_set<BITSET>& min_seps, int k,
                 const unordered_set<BITSET>& is_feasible_memo) {
    if (is_feasible_memo.count(C)) return true;
    if (min_seps.count(open_neighbors(g, C)) && close_neighbors(g, C).count() <= k + 1) return true;
    int min_C = get_min(C);
    for (auto& D : is_feasible_memo) {
        if (D.test(min_C) &&
            all_feasible(g, join(open_neighbors(g, C), open_neighbors(g, D)), C, min_seps, k, is_feasible_memo))
            return true;
    }
    BITSET S_ = open_neighbors(g, C);
    S_.set(min_C);
    return all_feasible(g, S_, C, min_seps, k, is_feasible_memo);
}

bool is_inbound(const Graph& g, const BITSET& C) {
    int min_C = get_min(C);
    auto N = open_neighbors(g, C);
    for (auto& conn : components(remove(g, N))) {
        if (open_neighbors(g, conn) == N && get_min(conn) < min_C) {
            return true;
        }
    }
    return false;
}

bool all_feasible(const Graph& g, const BITSET& S, const BITSET& C, const unordered_set<BITSET>& min_seps, int k,
                  const unordered_set<BITSET>& is_feasible_memo) {
    if (S.count() > k + 1) return false;
    for (auto& D : components(remove(g, S))) {
        if (is_subset(D, C)) {
            if (open_neighbors(g, D) == S) {
                if (!is_feasible(g, D, min_seps, k, is_feasible_memo)) return false;
            } else {
                // assert(is_inbound(g, D));
                if (!is_feasible_memo.count(D)) return false;
            }
        }
    }
    return true;
}

bool msdp(const Graph& g, const unordered_set<BITSET>& min_seps, int k) {
    vector<BITSET> inbounds;
    for (auto& s : min_seps) {
        vector<BITSET> C;
        vector<int> mins;
        for (auto& conn : components(remove(g, s))) {
            if (s != open_neighbors(g, conn)) continue;
            C.push_back(conn);
            mins.push_back(get_min(conn));
        }
        int nin = *min_element(mins.begin(), mins.end());
        assert(C.size() > 1);
        for (int i = 0; i < C.size(); i++) {
            if (mins[i] != nin) {
                inbounds.push_back(C[i]);
            }
        }
    }
    sort(inbounds.begin(), inbounds.end(), [](auto& a, auto& b) { return a.count() < b.count(); });
    unordered_set<BITSET> is_feasible_memo;
    for (auto& C : inbounds) {
        if (is_feasible(g, C, min_seps, k, is_feasible_memo)) is_feasible_memo.insert(C);
    }
    return is_feasible(g, g.nodes, min_seps, k, is_feasible_memo);
}

bool treewidth_des(const Graph& g, int k) {
    auto seps = list_exact(g, k);
    return msdp(g, unordered_set<BITSET>(seps.begin(), seps.end()), k);
}

int treewidth_exact(const Graph& g) {
    int k = 0;
    while (1) {
        if (treewidth_des(g, k)) return k;
        k++;
    }
}

int degeneracy(const Graph& g) {
    int n = g.n();
    unordered_set<int> visited;
    unordered_map<int, int> deg;
    vector<unordered_set<int>> D(n);
    FOR_EACH(v, g.nodes) {
        deg[v] = at(g.adj, v).count();
        D[deg[v]].insert(v);
    }
    int ret = 0;
    for (int _ = 0; _ < n; _++) {
        for (int i = 0; i < n; i++) {
            if (D[i].empty()) continue;
            ret = max(ret, i);
            int v = *D[i].begin();
            D[i].erase(v);
            visited.insert(v);
            FOR_EACH(w, at(g.adj, v)) {
                if (!visited.count(w)) {
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

// treewidth == max clique size - 1 == coloring number - 1 == degeneracy
int treewidth_chordal(const Graph& g) { return degeneracy(g); }

int treewidth_lb(const Graph& g) { return degeneracy(g); }

// treewidth heuristic by mindeg
int mindeg(Graph g) {
    Graph h(g);
    int n = g.n();
    unordered_set<int> erased;
    unordered_map<int, int> deg;
    vector<unordered_set<int>> D(n);
    FOR_EACH(v, g.nodes) {
        deg[v] = at(g.adj, v).count();
        D[deg[v]].insert(v);
    }
    auto has_edge = [&](int a, int b) { return at(g.adj, a).test(b); };
    auto erase_edge = [&](int a, int b) {
        if (!erased.count(a)) {
            D[deg[a]].erase(a);
            deg[a]--;
            D[deg[a]].insert(a);
        }
        if (!erased.count(b)) {
            D[deg[b]].erase(b);
            deg[b]--;
            D[deg[b]].insert(b);
        }
        at(g.adj, a).reset(b);
        at(g.adj, b).reset(a);
    };
    auto add_edge = [&](int a, int b) {
        D[deg[a]].erase(a);
        deg[a]++;
        D[deg[a]].insert(a);
        D[deg[b]].erase(b);
        deg[b]++;
        D[deg[b]].insert(b);
        at(g.adj, a).set(b);
        at(g.adj, b).set(a);
    };
    for (int _ = 0; _ < n; _++) {
        for (int i = 0; i < n; i++) {
            if (D[i].empty()) continue;
            int v = *D[i].begin();
            D[i].erase(v);
            erased.insert(v);
            vector<int> neighbors;
            FOR_EACH(w, at(g.adj, v)) {
                if (!erased.count(w)) {
                    erase_edge(v, w);
                    neighbors.push_back(w);
                }
            }
            for (int a : neighbors) {
                for (int b : neighbors) {
                    if (a < b && !has_edge(a, b)) {
                        add_edge(a, b);
                        h.add_edge(a, b);
                    }
                }
            }
            break;
        }
    }
    return treewidth_chordal(h);
}

// treewidth heuristic by minfill
int minfill(Graph g) {
    Graph h(g);
    int n = g.n();
    for (int _ = 0; _ < n; _++) {
        int v = min_fill_vertex(g);
        FOR_EACH(a, at(g.adj, v)) {
            FOR_EACH(b, at(g.adj, v)) {
                if (a < b && !at(h.adj, a).test(b)) {
                    g.add_edge(a, b);
                    h.add_edge(a, b);
                }
            }
        }
        g.remove_node(v);
    }
    return treewidth_chordal(h);
}

// compute near-optimal treewidth
int treewidth_ub(const Graph& g) {
    // return treewidth_exact(g);
    return min(mindeg(g), minfill(g));
}
