#pragma once
#include "../balanced_separator.cpp"
#include "../graph.cpp"

extern Graph treedepth_decomp(Graph g);

int treedepth_exact(const Graph& g) {
    Graph decomp = treedepth_decomp(g);
    return depth(decomp, decomp.root);
}

constexpr int max_n = 30;
constexpr int max_precompute_num = 500;

vector<BITSET> BLOCKS;
vector<int> BLOCK_TD;

void decompose(const Graph& g, double alpha, int max_size) {
    int min_size = (max_size + 1) / 2;
    if (g.n() <= max_size) {
        if (g.n() >= min_size) BLOCKS.push_back(g.nodes);
        return;
    }
    auto sep = GA(g, alpha);
    for (auto& C : components(remove(g, sep))) {
        decompose(induced(g, C), alpha, max_size);
    }
}

void init_blocks(const Graph& g) {
    double alpha = 0.5;
    int max_size = max_n;
    for (int _ = 0; _ < 5; _++) {
        for (int i = 0; i < max_precompute_num; i++) {
            decompose(g, alpha, max_size);
            alpha += 0.01;
            if (alpha > 0.8) alpha = 0.5;
        }
        if (BLOCKS.size() >= max_precompute_num) break;
        max_size *= 0.9;
    }
    sort(BLOCKS.begin(), BLOCKS.end());
    BLOCKS.erase(unique(BLOCKS.begin(), BLOCKS.end()), BLOCKS.end());
    sort(BLOCKS.begin(), BLOCKS.end(), [](auto& a, auto& b) { return a.count() > b.count(); });
    BLOCKS.resize(min((int)BLOCKS.size(), max_precompute_num));
    sort(BLOCKS.begin(), BLOCKS.end(), [](auto& a, auto& b) { return a.count() < b.count(); });

    BLOCK_TD.resize(BLOCKS.size());
    for (int i = 0; i < BLOCKS.size(); i++) {
        BLOCK_TD[i] = treedepth_exact(induced(g, BLOCKS[i]));
    }
}
