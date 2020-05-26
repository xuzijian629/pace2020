#pragma once
#include "graph.cpp"
#include "minimal_separator.cpp"

namespace std {
template <>
struct hash<BITSET> {
    inline size_t operator()(const BITSET& s) const { return get_hash(s); }
};
}  // namespace std

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
    BITSET visited;
    array<int, BITSET_MAX_SIZE> deg;
    array<BITSET, BITSET_MAX_SIZE> D;
    FOR_EACH(v, g.nodes) {
        deg[v] = at(g.adj, v).count();
        D[deg[v]].set(v);
    }
    int ret = 0;
    int n = g.n();
    for (int _ = 0; _ < n; _++) {
        for (int i = 0; i < n; i++) {
            if (!D[i].any()) continue;
            ret = max(ret, i);
            int v = D[i]._Find_first();
            D[i].reset(v);
            visited.set(v);
            FOR_EACH(w, at(g.adj, v)) {
                if (!visited.test(w)) {
                    assert(D[deg[w]].test(w));
                    D[deg[w]].reset(w);
                    deg[w]--;
                    D[deg[w]].set(w);
                }
            }
            break;
        }
    }
    return ret;
}

int minor_min_width(Graph g) {
    array<int, BITSET_MAX_SIZE> deg;
    array<BITSET, BITSET_MAX_SIZE> D;
    FOR_EACH(v, g.nodes) {
        deg[v] = at(g.adj, v).count();
        D[deg[v]].set(v);
    }

    // deg[v] <= deg[u], u and v are adjacent
    auto contract = [&](int v, int u) {
        // N(v) & N(u)
        BITSET adj_both = g.adj[v] & g.adj[u];
        // N(v) \ N[u]
        BITSET adj_v_only = g.adj[v] ^ adj_both;
        adj_v_only.reset(u);
        // modify u
        g.adj[u] |= adj_v_only;
        g.adj[u].reset(v);
        D[deg[u]].reset(u);
        deg[u] += adj_v_only.count();  // connect to adj_v_only
        deg[u]--;                      // remove v
        D[deg[u]].set(u);
        // modify adj_both
        FOR_EACH(w, adj_both) {
            g.adj[w].reset(v);
            D[deg[w]].reset(w);
            deg[w]--;  // remove v
            D[deg[w]].set(w);
        }
        // modify adj_v_only
        FOR_EACH(w, adj_v_only) {
            g.adj[w].reset(v);  // remove v
            g.adj[w].set(u);    // connect to u
        }
    };

    int n = g.n();
    int ret = 0;
    int d = 0;
    for (int _ = 0; _ < n - 1; _++) {
        while (!D[d].any()) {
            d++;
        }
        assert(d != 0);  // d cannot be 0 (do only n - 1 loops)
        ret = max(ret, d);
        int v = D[d]._Find_first();
        assert(at(g.adj, v).count() == d);
        D[d].reset(v);
        int nin = 1e9;
        int u = -1;
        FOR_EACH(w, at(g.adj, v)) {
            int common_neighbors = intersection(at(g.adj, v), at(g.adj, w)).count();
            if (common_neighbors < nin) {
                nin = common_neighbors;
                u = w;
            }
        }
        assert(u != -1);
        contract(v, u);
        d--;  // current minimum d may be reduced by 1
    }
    return ret;
}

// treewidth == max clique size - 1 == coloring number - 1 == degeneracy
int treewidth_chordal(const Graph& g) { return degeneracy(g); }

// treewidth heuristic by mindeg
int mindeg(Graph g) {
    Graph h(g);
    int n = g.n();
    BITSET erased;
    array<int, BITSET_MAX_SIZE> deg;
    array<BITSET, BITSET_MAX_SIZE> D;
    FOR_EACH(v, g.nodes) {
        deg[v] = at(g.adj, v).count();
        D[deg[v]].set(v);
    }
    auto has_edge = [&](int a, int b) { return at(g.adj, a).test(b); };
    auto erase_edge = [&](int a, int b) {
        if (!erased.test(a)) {
            D[deg[a]].reset(a);
            deg[a]--;
            D[deg[a]].set(a);
        }
        if (!erased.test(b)) {
            D[deg[b]].reset(b);
            deg[b]--;
            D[deg[b]].set(b);
        }
        at(g.adj, a).reset(b);
        at(g.adj, b).reset(a);
    };
    auto add_edge = [&](int a, int b) {
        D[deg[a]].reset(a);
        deg[a]++;
        D[deg[a]].set(a);
        D[deg[b]].reset(b);
        deg[b]++;
        D[deg[b]].set(b);
        at(g.adj, a).set(b);
        at(g.adj, b).set(a);
    };
    for (int _ = 0; _ < n; _++) {
        for (int i = 0; i < n; i++) {
            if (!D[i].any()) continue;
            int v = D[i]._Find_first();
            D[i].reset(v);
            erased.set(v);
            vector<int> neighbors;
            FOR_EACH(w, at(g.adj, v)) {
                if (!erased.test(w)) {
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
        auto X = at(g.adj, v);
        FOR_EACH(a, at(g.adj, v)) {
            at(g.adj, a) |= X;
            at(g.adj, a).reset(a);
            at(h.adj, a) |= X;
            at(h.adj, a).reset(a);
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
