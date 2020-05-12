#pragma once
#include <bits/stdc++.h>

#include "bitset.cpp"
using namespace std;

#ifndef BITSET_MAX_SIZE
#define BITSET_MAX_SIZE 500
#endif
using BITSET = bitset<BITSET_MAX_SIZE>;
// using BITSET = Bitset;
int MAX_NODE_SIZE;
#define FOR_EACH(v, bs) for (int v = (bs)._Find_first(); v < (bs).size(); v = (bs)._Find_next(v))

// array may be faster than unordered_map/vector
using ADJ = array<BITSET, BITSET_MAX_SIZE>;
// using ADJ = vector<BITSET>;
#define at(adj, i) adj[i]
// using ADJ = unordered_map<int, BITSET>;
// #define at(adj, i) adj.at(i)

struct Graph {
    int n() const { return nodes.count(); }
    int m() const {
        int ret = 0;
        FOR_EACH(v, nodes) ret += at(adj, v).count();
        return ret / 2;
    }
    // for rooted tree
    int root;
    BITSET nodes;
    ADJ adj;
    Graph() : root(-1) {}
    // Graph() : root(-1), adj(MAX_NODE_SIZE) {}
    Graph(int v) : root(v) { add_node(v); }
    // Graph(int v) : root(v), adj(MAX_NODE_SIZE) { add_node(v); }
    void add_node(int v) {
        if (!nodes.test(v)) {
            nodes.set(v);
        }
    }
    void add_edge(int a, int b) {
        add_node(a);
        add_node(b);
        if (!at(adj, a).test(b)) {
            at(adj, a).set(b);
            at(adj, b).set(a);
        }
    }
    void remove_node(int v) {
        assert(nodes.test(v));
        nodes.reset(v);
        FOR_EACH(a, adj[v]) { at(adj, a).reset(v); }
        at(adj, v).reset();
    }
    void remove_edge(int a, int b) {
        assert(at(adj, a).test(b));
        at(adj, a).reset(b);
        at(adj, b).reset(a);
    }
};

void print_graph(const Graph& g, bool to_one_indexed = false) {
    cout << "n=" << g.n() << ", m=" << g.m() << ", nodes={";
    FOR_EACH(v, g.nodes) cout << v + to_one_indexed << ",";
    cout << "}, edges={";
    FOR_EACH(a, g.nodes) {
        FOR_EACH(b, at(g.adj, a)) {
            if (a < b) {
                cout << "(" << a + to_one_indexed << "," << b + to_one_indexed << "),";
            }
        }
    }
    cout << "}" << endl;
}

int depth(const Graph& tree, int root) {
    int ret = 0;
    auto dfs = [&](auto& dfs, int v, int p, int d) -> void {
        ret = max(ret, d);
        FOR_EACH(s, at(tree.adj, v)) {
            if (s != p) {
                dfs(dfs, s, v, d + 1);
            }
        }
    };
    dfs(dfs, root, -1, 1);
    return ret;
}

void print_decomp(const Graph& decomp) {
    assert(decomp.m() == decomp.n() - 1);
    cout << depth(decomp, decomp.root) << endl;
    vector<int> par(decomp.n());
    auto dfs = [&](auto& dfs, int v, int p) -> void {
        par[v] = p;
        FOR_EACH(s, at(decomp.adj, v)) {
            if (s != p) dfs(dfs, s, v);
        }
    };
    dfs(dfs, decomp.root, -1);
    for (int v = 0; v < decomp.n(); v++) {
        cout << par[v] + 1 << '\n';
    }
}

Graph read_input() {
    string s;
    int n, m;
    cin >> s >> s >> n >> m;
    MAX_NODE_SIZE = n;
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
void merge(Graph& g1, const Graph& g2, int u, int v) {
    assert(g1.nodes.test(u) && g2.nodes.test(v));
    g1.nodes |= g2.nodes;
    FOR_EACH(a, g2.nodes) { g1.adj[a] = at(g2.adj, a); }
    g1.add_edge(u, v);
}

vector<BITSET> components(const Graph& g) {
    vector<BITSET> ret;
    BITSET not_visited = g.nodes;

    BITSET visited;
    auto bfs = [&](auto& bfs, const BITSET& cur) -> void {
        BITSET nxt;
        FOR_EACH(v, cur) { nxt |= at(g.adj, v) ^ (at(g.adj, v) & visited); }
        if (nxt.any()) {
            visited |= nxt;
            bfs(bfs, nxt);
        }
    };

    FOR_EACH(v, not_visited) {
        BITSET cur;
        visited.set(v);
        cur.set(v);
        bfs(bfs, cur);
        ret.push_back(visited);
        not_visited ^= visited;
        visited.reset();
    }
    return ret;
}

bool is_adjacent(const Graph& g, int u, int v) {
    assert(g.nodes.test(u) && g.nodes.test(v));
    return at(g.adj, u).test(v);
}

Graph induced(const Graph& g, const BITSET& S) {
    assert((g.nodes & S) == S);
    Graph ret(g);
    FOR_EACH(a, ret.nodes) {
        if (S.test(a)) {
            at(ret.adj, a) &= S;
        } else {
            at(ret.adj, a).reset();
        }
    }
    ret.nodes = S;
    return ret;
}

int longest_path_lb(const Graph &g) {
    struct priority_t {
        int deg = 1;
        int id = INT_MAX;
        bool operator<(const priority_t &another) const {
            if (deg == another.deg) return id < another.id;
            else if (deg == 1) return false;
            else if (another.deg == 1) return true;
            else return deg < another.deg; 
        }
    };
    set<priority_t> st;
    int n = g.n();
    if (n <= 3) return n;
    vector<int> degs(BITSET_MAX_SIZE);
    FOR_EACH(v, g.nodes) {
        degs[v] = g.adj[v].count();
        st.insert({degs[v], v});
    }
    int ret = 1;
    // try the first 2 nodes and last 2 nodes
    vector<priority_t> us(4);
    {
        set<priority_t>::iterator itr;
        itr = st.begin();
        us[0] = *itr;
        ++itr;
        us[1] = *itr;
        itr = st.end();
        --itr;
        us[2] = *itr;
        --itr;
        us[3] = *itr;
    }
    for (int k = 0; k < 4; ++k) {
        set<priority_t> st_buf = st;
        vector<int> degs_buf = degs;
        set<priority_t>::iterator itr = st_buf.find(us[k]);
        auto [deg_u, u] = *itr;
        vector<priority_t> nbs; // N(u), neighbors of u
        degs_buf[u] = deg_u = 0;
        st_buf.erase(itr); // remove u
        // decrement degree of each N(u)
        FOR_EACH(v, g.adj[u]) {
            itr = st_buf.find({degs_buf[v], v});
            st_buf.erase(itr);
            degs_buf[v]--;
            st_buf.insert({degs_buf[v], v});
            nbs.push_back({degs_buf[v], v});
        }
        // sort by degree in order: [0] [2 ... n] [1]
        sort(nbs.begin(), nbs.end());
        // greedy path from N(b) with dependency
        vector<int> vals(nbs.size());
        for (int i = 0; i < nbs.size(); ++i) {
            itr = st_buf.find({ degs_buf[nbs[i].id], nbs[i].id });
            // only when already used
            if (itr == st_buf.end()) {
                vals[i] = 0;
                continue;
            }
            while (true) {
                auto [deg_v, v] = *itr;
                st_buf.erase(itr);
                vals[i]++;
                degs_buf[v] = deg_v = 0;
                priority_t pr = {1, INT_MAX};
                // decrement degree of each N(v)
                FOR_EACH(w, g.adj[v]) {
                    if (degs_buf[w] == 0) continue;
                    itr = st_buf.find({degs_buf[w], w});
                    pr = min(pr, *itr);
                    st_buf.erase(itr);
                    degs_buf[w]--;
                    if (degs_buf[w] > 0) st_buf.insert({degs_buf[w], w});
                }
                pr.deg--;
                if (pr.deg == 0) {
                    if (pr.id != INT_MAX) vals[i]++;
                    break;
                }
                itr = st_buf.find(pr);
            }
        }
        sort(vals.begin(), vals.end(), greater<int>());
        int buf = 1;
        if (vals.size() >= 1) buf += vals[0];
        if (vals.size() >= 2) buf += vals[1];
        ret = max(ret, buf);
    }
    return ret;
}