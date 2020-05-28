#pragma once
#include "graph.cpp"
#include "minimal_separator.cpp"

// implementation of balanced separator heuristic
// Exact and Heuristic Methods for the Vertex Separator Problem
// Haeder Althoby, Mohamed Biha, and Andre Sesboue

random_device rnd;
int get_minimum_degree_v(const Graph& g) {
    int nin = 1e9;
    vector<int> mins;
    FOR_EACH(v, g.nodes) {
        int d = at(g.adj, v).count();
        if (d < nin) {
            nin = d;
            mins.clear();
            mins.push_back(v);
        } else if (d == nin) {
            mins.push_back(v);
        }
    }
    assert(nin != 1e9);
    return mins[rnd() % mins.size()];
}

BITSET GA(const Graph& g, double alpha = 0.667) {
    int a = get_minimum_degree_v(g);
    int n = g.n();
    int beta = max(1, (int)(n * alpha));
    BITSET A, C = open_neighbors(g, a);
    A.set(a);
    BITSET B = difference(g.nodes, join(A, C));
    auto new_a = [&]() {
        int nin = 1e9;
        vector<int> mins;
        FOR_EACH(i, difference(g.nodes, A)) {
            int s = intersection(open_neighbors(g, i), B).count();
            if (s < nin) {
                nin = s;
                mins.clear();
                mins.push_back(i);
            } else if (s == nin) {
                mins.push_back(i);
            }
        }
        assert(nin != 1e9);
        return mins[rnd() % mins.size()];
    };
    while (A.count() + C.count() <= n - beta) {
        int i = new_a();
        A.set(i);
        C = open_neighbors(g, A);
        B = difference(g.nodes, join(A, C));
    }
    return C;
}

Graph treedepth_heuristic(const Graph& g) {
    int n = g.n();
    if (n * (n - 1) / 2 == g.m()) {
        Graph decomp;
        vector<int> nodes;
        FOR_EACH(v, g.nodes) nodes.push_back(v);
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }
        return decomp;
    }
    BITSET sep = GA(g);
    Graph decomp;
    vector<int> nodes;
    FOR_EACH(v, sep) nodes.push_back(v);
    decomp.add_node(nodes[0]);
    decomp.root = nodes[0];
    for (int i = 1; i < nodes.size(); i++) {
        decomp.add_edge(nodes[i - 1], nodes[i]);
    }
    assert(g.nodes != sep);
    for (auto& C : components(remove(g, sep))) {
        auto subtree = treedepth_heuristic(induced(g, C));
        merge(decomp, subtree, nodes.back(), subtree.root);
    }
    return decomp;
}
