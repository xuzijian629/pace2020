#pragma once
#include "../balanced_separator.cpp"
#include "../graph.cpp"

extern Graph treedepth_decomp(Graph g);

int treedepth_exact(const Graph& g) {
    Graph decomp = treedepth_decomp(g);
    return depth(decomp, decomp.root);
}

constexpr int max_n = (BITSET_MAX_SIZE > 100 ? 40 : 30);
constexpr int min_n = max_n / 2;
constexpr int max_precompute_num = 500;

vector<BITSET> BLOCKS;
vector<int> BLOCK_TD;

void decompose(const Graph& g, double alpha) {
    if (g.n() <= max_n) {
        if (g.n() >= min_n) BLOCKS.push_back(g.nodes);
        return;
    }
    auto sep = GA(g, alpha);
    for (auto& C : components(remove(g, sep))) {
        decompose(induced(g, C), alpha);
    }
}

void init_blocks(const Graph& g) {
    double alpha = 0.5;
    for (int i = 0; i < max_precompute_num; i++) {
        decompose(g, alpha);
        alpha += 0.01;
        if (alpha > 0.8) alpha = 0.5;
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
