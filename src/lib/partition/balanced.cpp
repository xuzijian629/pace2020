#pragma once
#include "../balanced_separator.cpp"
#include "../graph.cpp"

extern Graph treedepth_decomp(Graph g);

int treedepth_exact(const Graph& g) {
    Graph decomp = treedepth_decomp(g);
    return depth(decomp, decomp.root);
}

vector<vector<BITSET>> BLOCKS_SIZED;
vector<vector<int>> TD_SIZED;
vector<BITSET> BLOCKS;
vector<int> BLOCK_TD;

void decompose(const Graph& g, double alpha, int max_n) {
    int min_n = max_n / 2;
    if (g.n() <= max_n) {
        if (g.n() >= min_n) BLOCKS_SIZED[g.nodes.count()].push_back(g.nodes);
        return;
    }
    auto sep = GA(g, alpha);
    for (auto& C : components(remove(g, sep))) {
        decompose(induced(g, C), alpha, max_n);
    }
}

void enumerate_blocks(const Graph& g) {
    int n = g.n();
    BLOCKS_SIZED.resize(n);
    TD_SIZED.resize(n);
    for (int max_n = 30; max_n < n * 0.8; max_n++) {
        double alpha = 0.5;
        for (int _ = 0; _ < 100; _++) {
            decompose(g, alpha, max_n);
            alpha += 0.01;
            if (alpha > 0.8) alpha = 0.5;
        }
    }
    for (int i = 0; i < n; i++) {
        sort(BLOCKS_SIZED[i].begin(), BLOCKS_SIZED[i].end());
        BLOCKS_SIZED[i].erase(unique(BLOCKS_SIZED[i].begin(), BLOCKS_SIZED[i].end()), BLOCKS_SIZED[i].end());
    }
}

void compute_tds(const Graph& g, int size, int& tl_rem_millis) {
    int n = BLOCKS_SIZED[size].size();
    if (n == 0) return;
    n = min(n, 100);
    BLOCKS_SIZED[size].resize(n);
    TD_SIZED[size].resize(n);
    for (int i = 0; i < n; i++) {
        auto start = chrono::steady_clock::now();
        TD_SIZED[size][i] = treedepth_exact(induced(g, BLOCKS_SIZED[size][0]));
        auto finish = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(finish - start).count();
        tl_rem_millis -= elapsed;
        if (tl_rem_millis < 0) return;
    }
}

void init_blocks(const Graph& g, int max_blocks_num = 500) {
    enumerate_blocks(g);
    int tl_rem_millis = 500 * 1000;
    for (int i = 0; i < BLOCKS_SIZED.size(); i++) {
        compute_tds(g, i, tl_rem_millis);
        if (tl_rem_millis < 0) break;
    }
    bool finish = false;
    for (int i = 0; i < BLOCKS_SIZED.size(); i++) {
        if (finish) break;
        for (int j = 0; j < BLOCKS_SIZED[i].size(); j++) {
            if (TD_SIZED[i][j]) {
                BLOCKS.push_back(BLOCKS_SIZED[i][j]);
                BLOCK_TD.push_back(TD_SIZED[i][j]);
            } else {
                finish = true;
                break;
            }
        }
    }

    reverse(BLOCKS.begin(), BLOCKS.end());
    reverse(BLOCK_TD.begin(), BLOCK_TD.end());

    BLOCKS.resize(min((int)BLOCKS.size(), max_blocks_num));
    BLOCK_TD.resize(min((int)BLOCK_TD.size(), max_blocks_num));

    reverse(BLOCKS.begin(), BLOCKS.end());
    reverse(BLOCK_TD.begin(), BLOCK_TD.end());
}
