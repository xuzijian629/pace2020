#pragma once
#include "tw.cpp"

// compute treedepth of dfs tree for lb
// https://atcoder.jp/contests/agc009/submissions/8539379
int td_lb_dfs_tree(const Graph& g) {
    array<int, BITSET_MAX_SIZE> mask = {};
    BITSET visited;
    int ret = 0;
    auto dfs = [&](auto& dfs, int v) -> void {
        visited.set(v);
        int lowest = 0;
        FOR_EACH(u, at(g.adj, v)) {
            if (visited.test(u)) continue;
            dfs(dfs, u);
            lowest |= (mask[u] & mask[v]);
            mask[v] |= mask[u];
        }
        int bit = 1 << (lowest == 0 ? 0 : 32 - __builtin_clz(lowest));
        while (mask[v] & bit) {
            bit <<= 1;
        }
        mask[v] &= ~(bit - 1);
        mask[v] |= bit;
        ret = max(ret, 31 - __builtin_clz(mask[v]));
    };
    dfs(dfs, g.nodes._Find_first());
    return ret;
}

int longest_path_lb(Graph g) {
    int n = g.n();
    if (n <= 3) return n;
    BITSET erased;
    array<unsigned, BITSET_MAX_SIZE> deg;
    int u;  // the node with smallest degree
    {
        array<BITSET, BITSET_MAX_SIZE> D;
        FOR_EACH(v, g.nodes) {
            deg[v] = g.adj[v].count();
            D[deg[v]].set(v);
        }
        // try the node with smallest degree
        for (size_t i = 1; i < n; ++i) {
            if (!D[i].any()) continue;
            u = D[i]._Find_first();
            break;
        }
    }
    deg[u] = 0;
    erased.set(u);
    FOR_EACH(v, g.adj[u]) {
        deg[v]--;
        g.adj[v].reset(u);
    }
    // save top two longest paths
    array<int, 2> top_lengths = {};
    FOR_EACH(nu, g.adj[u]) {
        // only when already used
        if (erased.test(nu)) continue;
        int v = nu;
        int length = 0;
        while (true) {
            length++;
            // end at singleton
            if (!g.adj[v].any()) {
                deg[v] = 0;
                erased.set(v);
                break;
            }
            int best_w = g.adj[v]._Find_first();
            unsigned best_d = deg[best_w];
            for (int w = g.adj[v]._Find_next(best_w); w < g.adj[v].size(); w = g.adj[v]._Find_next(w)) {
                // priority: [2, ..., n, 1], ==(-2)=> [0, 1, ..., n - 2, UINT_MAX]
                if (deg[w] - 2 < best_d - 2) {
                    best_w = w;
                    best_d = deg[w];
                }
            }
            deg[v] = 0;
            erased.set(v);
            FOR_EACH(w, g.adj[v]) {
                deg[w]--;
                g.adj[w].reset(v);
            }
            v = best_w;
        }
        // update top lengths
        if (top_lengths[1] < length) swap(top_lengths[1], length);
        if (top_lengths[0] < top_lengths[1]) swap(top_lengths[0], top_lengths[1]);
    }
    return 1 + top_lengths[0] + top_lengths[1];
}

int lb_n_m(const Graph& g) {
    int n = g.n(), m = g.m();
    int l = 1, r = n;
    while (l < r - 1) {
        int k = (l + r) / 2;
        if (2 * m <= (k - 1) * (2 * n - k)) {
            r = k;
        } else {
            l = k;
        }
    }
    return r;
}

int lb_n_d(const Graph& g) {
    int n = g.n();
    int d = 0;
    FOR_EACH(v, g.nodes) d = max(d, (int)at(g.adj, v).count());
    assert(d != 1);
    auto lb = [&](auto& lb, int n, int d) -> int {
        if (n == 0) return 0;
        return 1 + lb(lb, (n + d - 2) / d, d);
    };
    return lb(lb, n, d);
}

// return true if treedepth_lb(g) > lim
bool prune_by_td_lb(const Graph& g, int lim) {
    int n = g.n();
    if (n <= 2) return n > lim;
    if (treewidth_lb(g) + 1 > lim) return true;
    if (lim < 30 && (1 << lim) <= g.n()) {
        if (32 - __builtin_clz(longest_path_lb(g)) > lim) return true;
    }
    if (lb_n_m(g) > lim) return true;
    if (lb_n_d(g) > lim) return true;
    // if (td_lb_dfs_tree(g) > lim) return true;
    return false;
}
