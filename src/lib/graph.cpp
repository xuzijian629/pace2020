#pragma once
#include <bits/stdc++.h>
using namespace std;

struct Graph {
    int n, m;
    vector<vector<int>> adj;
    // 構築時に必要になるため、分解後のグラフでも入力グラフでの頂点番号を保持しておく
    vector<int> original_index;
    Graph() {}
    Graph(int n) : n(n), m(0), adj(n), original_index(n) { iota(original_index.begin(), original_index.end(), 0); }
    void add_edge(int a, int b) {
        assert(0 <= a && a < n);
        assert(0 <= b && b < n);
        adj[a].push_back(b);
        adj[b].push_back(a);
        m++;
    }
};

void print_graph(const Graph& g) {
    cout << "n=" << g.n << ", m=" << g.m << ", nodes={";
    for (int i = 0; i < g.n; i++) {
        cout << g.original_index[i] << ",";
    }
    cout << "}, edges={";
    for (int a = 0; a < g.n; a++) {
        for (int b : g.adj[a]) {
            if (a < b) {
                cout << "(" << g.original_index[a] << "," << g.original_index[b] << "),";
            }
        }
    }
    cout << "}" << endl;
}

Graph read_input() {
    string s;
    int n, m;
    cin >> s >> s >> n >> m;
    Graph ret(n);
    for (int i = 0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        a--, b--;
        ret.add_edge(a, b);
    }
    return ret;
}

// O(m)
// 木の深さを求める
int depth(const Graph& tree, int root = 0) {
    assert(root < tree.n);
    assert(tree.m == tree.n - 1);
    int ret = 0;
    auto dfs = [&](auto& dfs, int v, int p, int d) -> void {
        ret = max(ret, d);
        for (auto& s : tree.adj[v]) {
            if (s != p) {
                dfs(dfs, s, v, d + 1);
            }
        }
    };
    dfs(dfs, root, -1, 1);
    return ret;
}

// O(m)
// グラフ g から頂点 u を削除して得られるグラフを連結成分ごとに返す
// 頂点番号は各連結成分ごとに 0 から振り直されている
vector<Graph> remove(const Graph& g, int u) {
    vector<int> color(g.n, -1);
    auto dfs = [&](auto& dfs, int v, int c) -> void {
        color[v] = c;
        for (int s : g.adj[v]) {
            if (s == u) continue;
            if (color[s] == -1) dfs(dfs, s, c);
        }
    };
    int c = 0;
    for (int i = 0; i < g.n; i++) {
        if (i == u) continue;
        if (color[i] == -1) {
            dfs(dfs, i, c);
            c++;
        }
    }
    vector<int> node_cnt(c);
    for (int i = 0; i < g.n; i++) {
        if (i == u) continue;
        node_cnt[color[i]]++;
    }
    vector<Graph> ret(c);
    for (int i = 0; i < c; i++) ret[i] = Graph(node_cnt[i]);
    node_cnt.assign(c, 0);
    vector<int> new_node_id(g.n, -1);
    for (int a = 0; a < g.n; a++) {
        for (int b : g.adj[a]) {
            if (a == u || b == u) continue;
            if (a < b) {
                assert(color[a] == color[b]);
                if (new_node_id[a] == -1) {
                    new_node_id[a] = node_cnt[color[a]]++;
                }
                if (new_node_id[b] == -1) {
                    new_node_id[b] = node_cnt[color[b]]++;
                }
                ret[color[a]].add_edge(new_node_id[a], new_node_id[b]);
            }
        }
    }
    for (int i = 0; i < g.n; i++) {
        // 孤立点の id は 0
        if (new_node_id[i] == -1) new_node_id[i] = 0;
    }
    for (int i = 0; i < g.n; i++) {
        if (i == u) continue;
        ret[color[i]].original_index[new_node_id[i]] = g.original_index[i];
    }
    return ret;
}

// O(m)
// root (original index) と、各 subtree の根を結んだ新たな木をつくる
Graph merge_subtrees(int root, const vector<Graph>& subtrees) {
    int n = 1;
    for (auto& g : subtrees) {
        n += g.n;
    }
    Graph ret(n);
    ret.original_index[0] = root;
    n = 1;
    for (auto& g : subtrees) {
        ret.add_edge(0, n);
        for (int a = 0; a < g.n; a++) {
            ret.original_index[n + a] = g.original_index[a];
            for (int b : g.adj[a]) {
                if (a < b) {
                    ret.add_edge(n + a, n + b);
                }
            }
        }
        n += g.n;
    }
    assert(ret.m == ret.n - 1);
    return ret;
}

// treedepth decomposition を出力する
void print_decomp(const Graph& decomp) {
    assert(decomp.m == decomp.n - 1);
    cout << depth(decomp) << endl;
    vector<int> par(decomp.n);
    auto dfs = [&](auto& dfs, int v, int p) -> void {
        if (p == -1) {
            par[decomp.original_index[v]] = -1;
        } else {
            par[decomp.original_index[v]] = decomp.original_index[p];
        }
        for (auto& s : decomp.adj[v]) {
            if (s != p) {
                dfs(dfs, s, v);
            }
        }
    };
    dfs(dfs, 0, -1);
    for (int i = 0; i < decomp.n; i++) {
        cout << par[i] + 1 << '\n';
    }
}
