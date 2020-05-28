#pragma once
#include "flow.cpp"

BITSET fake_min_cut(const Graph& g, int u, int v) {
    vector<int> du(BITSET_MAX_SIZE, 1e9), dv(BITSET_MAX_SIZE, 1e9);
    {
        queue<int> que;
        que.emplace(u);
        du[u] = 0;
        while (!que.empty()) {
            int t = que.front();
            que.pop();
            FOR_EACH(a, at(g.adj, t)) {
                if (du[t] + 1 < du[a]) {
                    du[a] = du[t] + 1;
                    que.push(a);
                }
            }
        }
    }
    {
        queue<int> que;
        que.emplace(v);
        dv[v] = 0;
        while (!que.empty()) {
            int t = que.front();
            que.pop();
            FOR_EACH(a, at(g.adj, t)) {
                if (dv[t] + 1 < dv[a]) {
                    dv[a] = dv[t] + 1;
                    que.push(a);
                }
            }
        }
    }
    BITSET U;
    FOR_EACH(a, g.nodes) {
        if (du[a] < dv[a]) U.set(a);
    }
    BITSET V = difference(g.nodes, U);
    return open_neighbors(g, V);
}

BITSET fake_random_min_cut(const Graph& g) {
    assert(d_flow_init);
    vector<int> vs;
    FOR_EACH(v, g.nodes) vs.push_back(v);
    int v = vs[rnd() % vs.size()];
    vector<int> ds;
    for (int u : vs) ds.push_back(d_flow[v][u]);
    for (int i = 1; i < ds.size(); i++) {
        ds[i] += ds[i - 1];
    }
    int r = rnd() % ds.back() + 1;
    int idx = -1;
    for (int i = 0; i < ds.size(); i++) {
        if (ds[i] >= r) {
            idx = i;
            break;
        }
    }
    assert(idx != -1);
    return fake_min_cut(g, v, vs[idx]);
}
