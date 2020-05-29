#pragma once
#include "../graph.cpp"
#include "../minimal_separator.cpp"

template <typename flow_t>
struct Dinic {
    const flow_t INF;

    struct edge {
        int to;
        flow_t cap;
        int rev;
        bool isrev;
        int idx;
    };

    vector<vector<edge>> graph;
    vector<int> min_cost, iter;

    Dinic(int V) : INF(numeric_limits<flow_t>::max()), graph(V) {}

    void add_edge(int from, int to, flow_t cap, int idx = -1) {
        graph[from].emplace_back((edge){to, cap, (int)graph[to].size(), false, idx});
        graph[to].emplace_back((edge){from, 0, (int)graph[from].size() - 1, true, idx});
    }

    bool bfs(int s, int t) {
        min_cost.assign(graph.size(), -1);
        queue<int> que;
        min_cost[s] = 0;
        que.push(s);
        while (!que.empty() && min_cost[t] == -1) {
            int p = que.front();
            que.pop();
            for (auto &e : graph[p]) {
                if (e.cap > 0 && min_cost[e.to] == -1) {
                    min_cost[e.to] = min_cost[p] + 1;
                    que.push(e.to);
                }
            }
        }
        return min_cost[t] != -1;
    }

    flow_t dfs(int idx, const int t, flow_t flow) {
        if (idx == t) return flow;
        for (int &i = iter[idx]; i < graph[idx].size(); i++) {
            edge &e = graph[idx][i];
            if (e.cap > 0 && min_cost[idx] < min_cost[e.to]) {
                flow_t d = dfs(e.to, t, min(flow, e.cap));
                if (d > 0) {
                    e.cap -= d;
                    graph[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    }

    flow_t max_flow(int s, int t) {
        flow_t flow = 0;
        while (bfs(s, t)) {
            iter.assign(graph.size(), 0);
            flow_t f = 0;
            while ((f = dfs(s, t, INF)) > 0) flow += f;
        }
        return flow;
    }

    void output() {
        for (int i = 0; i < graph.size(); i++) {
            for (auto &e : graph[i]) {
                if (e.isrev) continue;
                auto &rev_e = graph[e.to][e.rev];
                cout << i << "->" << e.to << " (flow: " << rev_e.cap << "/" << e.cap + rev_e.cap << ")" << endl;
            }
        }
    }
};

BITSET min_cut(const Graph &g, int u, int v) {
    assert(g.nodes.test(u) && g.nodes.test(v));
    Dinic<int> f(BITSET_MAX_SIZE);
    FOR_EACH(v, g.nodes) {
        FOR_EACH(u, at(g.adj, v)) { f.add_edge(u, v, 1); }
    }
    f.max_flow(u, v);
    BITSET U;
    U.set(u);
    queue<int> que;
    que.push(u);
    while (!que.empty()) {
        int t = que.front();
        que.pop();
        for (auto &e : f.graph[t]) {
            if (e.isrev) continue;
            if (e.cap && !U.test(e.to)) {
                U.set(e.to);
                que.push(e.to);
            }
        }
    }
    // open_neighbors(g, U) よりこっちのほうが良さそう？
    BITSET V = difference(g.nodes, U);
    return open_neighbors(g, V);
}

bool d_flow_init;
array<array<int, BITSET_MAX_SIZE>, BITSET_MAX_SIZE> d_flow;
void prepare_d_flow(const Graph &g) {
    for (int i = 0; i < BITSET_MAX_SIZE; i++) {
        for (int j = 0; j < BITSET_MAX_SIZE; j++) {
            d_flow[i][j] = 1e9;
        }
    }
    FOR_EACH(v, g.nodes) {
        FOR_EACH(u, at(g.adj, v)) { d_flow[u][v] = 1; }
    }
    for (int k = 0; k < BITSET_MAX_SIZE; k++) {
        for (int i = 0; i < BITSET_MAX_SIZE; i++) {
            for (int j = 0; j < BITSET_MAX_SIZE; j++) {
                d_flow[i][j] = min(d_flow[i][j], d_flow[i][k] + d_flow[k][j]);
            }
        }
    }
    d_flow_init = true;
}

BITSET random_min_cut(const Graph &g) {
    assert(d_flow_init);
    vector<pair<int, int>> ps;
    vector<int> ds;
    FOR_EACH(u, g.nodes) {
        FOR_EACH(v, g.nodes) {
            if (u < v) {
                if (rnd() & 1) {
                    ps.emplace_back(u, v);
                } else {
                    ps.emplace_back(v, u);
                }
                ds.push_back(d_flow[u][v]);
            }
        }
    }
    for (int i = 1; i < ds.size(); i++) {
        ds[i] += ds[i - 1];
        assert(ds[i] < 1e9);
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
    return min_cut(g, ps[idx].first, ps[idx].second);
}
