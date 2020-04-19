#pragma once
#include <bits/stdc++.h>
using namespace std;

struct Graph {
    int n, m;
    // for rooted tree
    int root;
    unordered_set<int> nodes;
    unordered_map<int, unordered_set<int>> adj;
    Graph() : n(0), m(0), root(-1) {}
    Graph(int v) : n(0), m(0), root(v) { add_node(v); }
    void add_node(int v) {
        if (!adj.count(v)) {
            n++;
            adj[v];
            nodes.insert(v);
        }
    }
    void add_edge(int a, int b) {
        add_node(a);
        add_node(b);
        if (!adj[a].count(b)) {
            m++;
            adj[a].insert(b);
            adj[b].insert(a);
        }
    }
    void remove_node(int v) {
        n--;
        assert(adj.count(v));
        for (int a : adj[v]) {
            adj[a].erase(v);
            m--;
        }
        adj.erase(v);
        nodes.erase(v);
    }
    void remove_edge(int a, int b) {
        assert(adj[a].count(b) && adj[b].count(a));
        adj[a].erase(b);
        adj[b].erase(a);
        m--;
    }
};

void print_graph(const Graph& g) {
    cout << "n=" << g.n << ", m=" << g.m << ", nodes={";
    for (int v : g.nodes) {
        cout << v << ",";
    }
    cout << "}, edges={";
    for (int a : g.nodes) {
        for (int b : g.adj.at(a)) {
            if (a < b) {
                cout << "(" << a << "," << b << "),";
            }
        }
    }
    cout << "}" << endl;
}

int depth(const Graph& tree, int root) {
    int ret = 0;
    auto dfs = [&](auto& dfs, int v, int p, int d) -> void {
        ret = max(ret, d);
        for (auto& s : tree.adj.at(v)) {
            if (s != p) {
                dfs(dfs, s, v, d + 1);
            }
        }
    };
    dfs(dfs, root, -1, 1);
    return ret;
}

void print_decomp(const Graph& decomp) {
    assert(decomp.m == decomp.n - 1);
    cout << depth(decomp, decomp.root) << endl;
    vector<int> par(decomp.n);
    auto dfs = [&](auto& dfs, int v, int p) -> void {
        par[v] = p;
        for (auto& s : decomp.adj.at(v)) {
            if (s != p) dfs(dfs, s, v);
        }
    };
    dfs(dfs, decomp.root, -1);
    for (int v = 0; v < decomp.n; v++) {
        cout << par[v] + 1 << '\n';
    }
}

Graph read_input() {
    string s;
    int n, m;
    cin >> s >> s >> n >> m;
    Graph ret;
    for (int v = 0; v < n; v++) ret.add_node(v);
    for (int i = 0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        a--, b--;
        ret.add_edge(a, b);
    }
    return ret;
}

// merge g2 into g1
void merge(Graph& g1, Graph& g2, int u, int v) {
    assert(g1.nodes.count(u) && g2.nodes.count(v));
    int root = g1.root;
    if (g1.n < g2.n) swap(g1, g2);
    for (int a : g2.nodes) {
        for (int b : g2.adj.at(a)) {
            if (a < b) g1.add_edge(a, b);
        }
    }
    g1.add_edge(u, v);
    g1.root = root;
}

vector<Graph> components(const Graph& g) {
    int color_cnt = 0;
    unordered_map<int, int> color(g.n);
    auto dfs = [&](auto& dfs, int v, int c) -> void {
        color[v] = c;
        for (auto& s : g.adj.at(v)) {
            if (!color.count(s)) dfs(dfs, s, c);
        }
    };
    for (int v : g.nodes) {
        if (!color.count(v)) dfs(dfs, v, color_cnt++);
    }
    vector<Graph> ret(color_cnt);
    for (int a : g.nodes) {
        ret[color[a]].add_node(a);
        for (int b : g.adj.at(a)) {
            if (a < b) {
                assert(color[a] == color[b]);
                ret[color[a]].add_edge(a, b);
            }
        }
    }
    return ret;
}
