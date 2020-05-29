#pragma once
#include "../balanced_separator.cpp"
#include "../graph.cpp"
#include "fakeflow.cpp"
#include "flow.cpp"

extern Graph treedepth_decomp(Graph g, bool use_block);

int treedepth_exact(const Graph& g) {
    Graph decomp = treedepth_decomp(g, false);
    return depth(decomp, decomp.root);
}

constexpr int tl_preprocess = 200000;
constexpr int precompute_iter = 200;
// サイズによって変えたほうがいい？
constexpr int blocks_max = 200;
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

// blocks から blocks_max 個の、できる限り共通部分が少ないような block の集合を選ぶ
vector<BITSET> take_good_blocks(const vector<BITSET>& blocks) {
    assert(blocks.size() > blocks_max);
    vector<BITSET> ret(blocks.begin(), blocks.begin() + blocks_max);
    array<int, BITSET_MAX_SIZE> sum = {};
    vector<bool> used(blocks.size());
    for (int i = 0; i < blocks_max; i++) used[i] = true;
    for (auto& b : ret) {
        FOR_EACH(v, b) sum[v]++;
    }
    for (int iter = 0; iter < 100; iter++) {
        int nax = -1;
        vector<int> worst;
        for (int i = 0; i < blocks_max; i++) {
            int cnt = 0;
            FOR_EACH(v, ret[i]) cnt += sum[v];
            if (cnt > nax) {
                nax = cnt;
                worst.clear();
                worst.push_back(i);
            } else if (cnt == nax) {
                worst.push_back(i);
            }
        }
        assert(nax != -1);
        auto rem = worst[rnd() % worst.size()];
        FOR_EACH(v, ret[rem]) sum[v]--;
        used[rem] = false;
        int nin = 1e9;
        vector<int> best;
        for (int i = 0; i < blocks.size(); i++) {
            if (used[i]) continue;
            int cnt = 0;
            FOR_EACH(v, blocks[i]) cnt += sum[v];
            if (cnt < nin) {
                nin = cnt;
                best.clear();
                best.push_back(i);
            } else if (cnt == nin) {
                best.push_back(i);
            }
        }
        assert(nin != 1e9);
        auto add = best[rnd() % best.size()];
        FOR_EACH(v, blocks[add]) sum[v]++;
        used[add] = true;
        ret[rem] = blocks[add];
    }
    return ret;
}

void gen_blocks(const Graph& g, int nax) {
    for (int max_n = 5; max_n <= nax; max_n++) {
        for (int i = 0; i < precompute_iter; i++) {
            int min_n = max_n / 2;
            decompose(g, min_n, max_n);
        }
    }
    for (int i = 0; i <= nax; i++) {
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
            if (uniq.size() > blocks_max) uniq = take_good_blocks(uniq);
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
    int nax = min(70, n / 2);
    prepare_d_flow(g);
    gen_blocks(g, nax);

    for (int i = 0; i <= nax; i++) {
        int sz = BLOCKS[i].size();
        if (!sz) continue;
        auto start = chrono::steady_clock::now();
        vector<pair<BITSET, int>> tmp;
        for (int j = 0; j < sz; j++) {
            int d = treedepth_exact(induced(g, BLOCKS[i][j]));
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
