/*
愚直DP解法
*/

#include <bits/stdc++.h>

#include "lib/graph.cpp"

using namespace std;

int node_set(const Graph& g) {
    int ret = 0;
    for (int i = 0; i < g.n; i++) {
        ret |= 1 << g.original_index[i];
    }
    return ret;
}

// treedepth decomposition を求める
// 頂点 0 を根とする根付き木
Graph solve(const Graph& g, vector<Graph>& memo) {
    int key = node_set(g);
    if (memo[key].n) return memo[key];
    if (g.n == 1) return g;
    int nin = 1e9;
    int best_to_remove = -1;
    vector<Graph> best_subtrees;
    for (int i = 0; i < g.n; i++) {
        vector<Graph> ss = remove(g, i);
        int nax = 0;
        vector<Graph> subtrees;
        for (const Graph& s : ss) {
            Graph subtree = solve(s, memo);
            int d = depth(subtree);
            nax = max(nax, d);
            subtrees.push_back(subtree);
        }
        assert(nax != 0);
        if (nax < nin) {
            nin = nax;
            swap(best_subtrees, subtrees);
            best_to_remove = g.original_index[i];
        }
    }
    assert(best_to_remove != -1);
    Graph decomp = merge_subtrees(best_to_remove, best_subtrees);
    assert(decomp.m == decomp.n - 1);
    return memo[key] = decomp;
}

int main() {
    Graph g = read_input();
    vector<Graph> memo(1 << g.n);
    Graph decomp = solve(g, memo);
    print_decomp(decomp);
}
