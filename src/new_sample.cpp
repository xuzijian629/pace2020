/*
愚直DP解法
sample.cpp と同じアルゴリズムだが、隣接リストをより操作しやすい形で持っている
実行速度はやや劣るものの、アルゴリズムを書き進めやすいと思う
*/

#include <bits/stdc++.h>

#include "lib/new_graph.cpp"

using namespace std;

Graph memo[1 << 20];

int node_set(const Graph& g) {
    int ret = 0;
    for (int v : g.nodes) {
        ret |= 1 << v;
    }
    return ret;
}

Graph solve(const Graph& g) {
    int key = node_set(g);
    if (memo[key].n) return memo[key];
    if (g.n == 1) {
        Graph cp(g);
        cp.root = *g.nodes.begin();
        return memo[key] = cp;
    }
    int nin = 1e9;
    int best_to_remove = -1;
    vector<Graph> best_subtrees;
    for (int v : g.nodes) {
        Graph cp(g);
        cp.remove_node(v);
        vector<Graph> comps = components(cp);
        int nax = 0;
        vector<Graph> subtrees;
        for (Graph& s : comps) {
            assert(s.n);
            Graph subtree = solve(s);
            int d = depth(subtree, subtree.root);
            nax = max(nax, d);
            subtrees.push_back(subtree);
        }
        assert(nax);
        if (nax < nin) {
            nin = nax;
            swap(best_subtrees, subtrees);
            best_to_remove = v;
        }
    }
    assert(best_to_remove != -1);
    Graph decomp(best_to_remove);
    for (Graph& g : best_subtrees) {
        merge(decomp, g, decomp.root, g.root);
    }
    return memo[key] = decomp;
}

int main() {
    Graph g = read_input();
    Graph decomp = solve(g);
    print_decomp(decomp);
}