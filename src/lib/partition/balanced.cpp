#pragma once
#include "../balanced_separator.cpp"
#include "../graph.cpp"
#include "fakeflow.cpp"
#include "flow.cpp"

extern Graph treedepth_decomp(Graph g, int use_block_max_size);

int treedepth_exact(const Graph& g, int use_block_max_size) {
    Graph decomp = treedepth_decomp(g, use_block_max_size);
    return depth(decomp, decomp.root);
}

constexpr int tl_preprocess = 600000;
constexpr int tl_enumerate = 60000;
int min_block_size = 1e9;

array<vector<BITSET>, BITSET_MAX_SIZE> BLOCKS;
array<vector<int>, BITSET_MAX_SIZE> BLOCK_TD;
array<int, BITSET_MAX_SIZE> NEXT_BLOCK;

void decompose(const Graph& g, int min_n, int max_n) {
    int n = g.n();
    if (n <= max_n) {
        if (n >= min_n) BLOCKS[n].push_back(g.nodes);
        return;
    }
    BITSET sep;
    if (rnd() & 1) {
        double alpha = 0.1 + 0.8 * (rnd() % 100) / 100;
        sep = GA(g, alpha);
    } else {
        // auto sep = random_min_cut(g);
        sep = fake_random_min_cut(g);
    }
    for (auto& C : components(remove(g, sep))) {
        decompose(induced(g, C), min_n, max_n);
    }
}

bool is_connected(const Graph& g) { return components(g).size() == 1; }

void gen_blocks(const Graph& g, int nax, int tl) {
    while (tl > 0) {
        auto start = chrono::steady_clock::now();
        for (int max_n = 5; max_n <= nax; max_n++) {
            int min_n = max_n / 2;
            for (int i = 0; i < max_n; i++) {
                decompose(g, min_n, max_n);
            }
        }
        auto finish = chrono::steady_clock::now();
        if (BLOCKS[nax / 2].size() > 300) break;
        tl -= chrono::duration_cast<chrono::milliseconds>(finish - start).count();
    }
    for (int i = 1; i <= nax; i++) {
        int sz = BLOCKS[i].size();
        if (sz) {
            cerr << "size " << i << ": " << sz << " found ";
            vector<BITSET> uniq;
            unordered_set<BITSET> ss;
            for (auto& b : BLOCKS[i]) {
                if (ss.count(b)) continue;
                assert(is_connected(induced(g, b)));
                ss.insert(b);
                uniq.push_back(b);
            }
            cerr << "(" << ss.size() << " unique)" << endl;
            min_block_size = min(min_block_size, i);
            // ソートしないほうが混ざってよさそう？
            int blocks_max = min(200, 1000 * BITSET_MAX_SIZE / (i * i));
            uniq.resize(min((int)uniq.size(), blocks_max));
            BLOCKS[i] = uniq;
        }
    }

    for (int i = 0; i < BITSET_MAX_SIZE; i++) NEXT_BLOCK[i] = -1;
    for (int i = min_block_size; i <= nax; i++) {
        int nxt = i + 1;
        while (BLOCKS[nxt].empty() && nxt <= nax) nxt++;
        if (nxt <= nax) {
            NEXT_BLOCK[i] = nxt;
        }
    }
}

void init_blocks(const Graph& g, int tl_millis) {
    int n = g.n();
    int nax = min(100, int(n * 0.9));
    prepare_d_flow(g);
    gen_blocks(g, nax, tl_enumerate);

    for (int i = 1; i <= nax; i++) {
        int sz = BLOCKS[i].size();
        if (!sz) continue;
        auto start = chrono::steady_clock::now();
        vector<pair<BITSET, int>> tmp;
        for (int j = 0; j < sz; j++) {
            int d = treedepth_exact(induced(g, BLOCKS[i][j]), i - 1);
            tmp.emplace_back(BLOCKS[i][j], d);
        }
        sort(tmp.begin(), tmp.end(), [](auto& a, auto& b) { return a.second > b.second; });
        BLOCK_TD[i].resize(sz);
        for (int j = 0; j < sz; j++) {
            BLOCKS[i][j] = tmp[j].first;
            BLOCK_TD[i][j] = tmp[j].second;
        }
        auto finish = chrono::steady_clock::now();
        tl_millis -= chrono::duration_cast<chrono::milliseconds>(finish - start).count();
        if (tl_millis < 0) {
            for (int j = i; j <= nax; j++) {
                NEXT_BLOCK[j] = -1;
            }
            break;
        }
    }
}
