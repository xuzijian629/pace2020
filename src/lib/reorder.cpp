#include "graph.cpp"

Graph reorder(const Graph& g, const vector<int>& to) {
    Graph ret;
    int n = g.n();
    for (int i = 0; i < n; i++) {
        assert(g.nodes.test(i));
        ret.add_node(to[i]);
        FOR_EACH(j, at(g.adj, i)) { ret.add_edge(to[i], to[j]); }
    }
    if (g.root != -1) ret.root = to[g.root];
    return ret;
}

pair<vector<int>, vector<int>> find_good_order(const Graph& g) {
    int n = g.n();
    vector<pair<int, int>> deg;
    for (int i = 0; i < n; i++) {
        deg.emplace_back(at(g.adj, i).count(), i);
    }
    sort(deg.rbegin(), deg.rend());
    vector<int> to(n), back(n);
    for (int i = 0; i < n; i++) {
        int v = deg[i].second;
        to[v] = i;
        back[i] = v;
    }
    return make_pair(to, back);
}
