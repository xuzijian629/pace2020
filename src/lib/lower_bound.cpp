#pragma once
#include "tw.cpp"

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
    array<int, 2> top_lengths = {0, 0};
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
int treedepth_lb(const Graph& g) {
    int ret = treewidth_lb(g) + 1;
    if (g.n() > 2) {
        ret = max(ret, 32 - __builtin_clz(longest_path_lb(g)));
    }
    return ret;
}
