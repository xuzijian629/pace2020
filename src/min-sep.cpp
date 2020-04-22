/*
minimal separator 列挙による解法
*/

#include "lib/minimal_separator.cpp"

using namespace std;

// treedepth k 以下の分解を作ることができるか
// 不可能なときは empty なグラフを返す
Graph solve(const Graph& g, int k) {
    // separator が存在しない <=> g が完全グラフ
    if (g.n * (g.n - 1) / 2 == g.m) {
        if (g.n > k) return Graph();
        Graph decomp;
        vector<int> nodes(g.nodes.begin(), g.nodes.end());
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }
        return decomp;
    }

    auto seps = list_exact(g, k);
    if (seps.empty()) return Graph();
    for (auto& s : seps) {
        Graph decomp;
        vector<int> nodes(s.begin(), s.end());
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }

        vector<Graph> subtrees;
        bool ok = true;
        for (auto& C : components(remove(g, s))) {
            Graph subtree = solve(C, k - s.size());
            if (!subtree.n) {
                ok = false;
                break;
            }
            subtrees.push_back(subtree);
        }
        if (!ok) continue;
        for (auto& subtree : subtrees) {
            merge(decomp, subtree, nodes.back(), subtree.root);
        }
        return decomp;
    }
    return Graph();
}

int main() {
    Graph g = read_input();
    for (int k = 1;; k++) {
        Graph decomp = solve(g, k);
        if (decomp.n) {
            print_decomp(decomp);
            break;
        }
    }
}
