#pragma once
#include "graph.cpp"
random_device rnd;

// 頂点数が k のパスを含むか
bool contains_path(const Graph& g, int k, int trial = 2, int max_fail = 2) {
    if (g.n < k) return false;
    if (k <= 3) {
        for (int a : g.nodes)
            if (g.adj.at(a).size() >= 2) return true;
    }
    // 乱択で伸ばしていくやつ書いたけどかなり遅い
    return false;
}

// 明らかに td(g) > k となってしまう場合 true を返す
bool terminate_by_lb(const Graph& g, int k) {
    if (k < 10 && contains_path(g, 1 << k)) return true;
    return false;
}
