#include "graph.cpp"

extern Graph treedepth_decomp(Graph g);

int treedepth_exact(const Graph& g) {
    Graph decomp = treedepth_decomp(g);
    return depth(decomp, decomp.root);
}

constexpr int min_n = 20;
constexpr int max_n = 30;
constexpr int max_precompute_num = 500;

vector<BITSET> BLOCKS;
vector<int> BLOCK_TD;
vector<vector<int>> all_dist;

void prepare_dist(const Graph& g) {
    int n = g.n();
    all_dist.resize(n);
    for (int i = 0; i < n; i++) {
        all_dist[i].assign(n, 1e9);
    }
    for (int a = 0; a < n; a++) {
        FOR_EACH(b, at(g.adj, a)) { all_dist[a][b] = 1; }
    }
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                all_dist[i][j] = min(all_dist[i][j], all_dist[i][k] + all_dist[k][j]);
            }
        }
    }
}

vector<BITSET> kmeans(const Graph& g, int k, int num_iter = 5) {
    assert(!all_dist.empty());
    int n = g.n();
    random_device rnd;
    vector<int> centers(k);
    using T = pair<int, int>;
    // init center
    {
        int v = rnd() % n;
        centers[0] = v;
        for (int i = 1; i < k; i++) {
            vector<int> dist(n, 1e9);
            priority_queue<T, vector<T>, greater<>> que;
            for (int j = 0; j < i; j++) {
                dist[centers[j]] = 0;
                que.emplace(0, centers[j]);
            }
            while (!que.empty()) {
                auto top = que.top();
                que.pop();
                if (dist[top.second] < top.first) continue;
                FOR_EACH(a, at(g.adj, top.second)) {
                    if (top.first + 1 < dist[a]) {
                        dist[a] = top.first + 1;
                        que.emplace(dist[a], a);
                    }
                }
            }
            vector<int> dist2(n);
            for (int i = 0; i < n; i++) {
                dist2[i] = dist[i] * dist[i];
            }
            int dist2_sum = accumulate(dist2.begin(), dist2.end(), 0);
            int r = rnd() % dist2_sum + 1;
            int sum = 0;
            bool found = false;
            for (int j = 0; j < n; j++) {
                sum += dist2[j];
                if (sum >= r) {
                    found = true;
                    centers[i] = j;
                    break;
                }
            }
            assert(found);
        }
    }

    // clustering
    for (int iter = 0;; iter++) {
        vector<int> nearest(n);
        vector<int> dist(n, 1e9);
        priority_queue<T, vector<T>, greater<>> que;
        for (int i = 0; i < k; i++) {
            nearest[centers[i]] = i;
            dist[centers[i]] = 0;
            que.emplace(0, centers[i]);
        }
        while (!que.empty()) {
            auto top = que.top();
            que.pop();
            if (dist[top.second] < top.first) continue;
            FOR_EACH(a, at(g.adj, top.second)) {
                if (top.first + 1 < dist[a]) {
                    dist[a] = top.first + 1;
                    que.emplace(dist[a], a);
                    nearest[a] = nearest[top.second];
                }
            }
        }
        vector<vector<int>> group(k);
        for (int i = 0; i < n; i++) {
            group[nearest[i]].push_back(i);
        }
        if (iter == num_iter) {
            vector<BITSET> ret(k);
            for (int i = 0; i < k; i++) {
                for (int a : group[i]) {
                    ret[i].set(a);
                }
            }
            return ret;
        }
        for (int i = 0; i < k; i++) {
            int nin = 1e9;
            int nxt_center = -1;
            for (int a : group[i]) {
                int sum = 0;
                for (int b : group[i]) {
                    if (a != b) sum += all_dist[a][b] * all_dist[a][b];
                }
                if (sum < nin) {
                    nin = sum;
                    nxt_center = a;
                }
            }
            assert(nxt_center != -1);
            centers[i] = nxt_center;
        }
    }
}

void init_blocks(const Graph& g) {
    prepare_dist(g);
    int n = g.n();
    int k = max(2, (n + min_n - 1) / min_n);
    bool finish = false;
    for (int i = 0; i < 2 * max_precompute_num; i++) {
        if (finish) break;
        auto blocks = kmeans(g, k);
        for (auto& C : blocks) {
            int size = C.count();
            if (min_n <= size && size <= max_n) {
                BLOCKS.push_back(C);
                if (BLOCKS.size() == max_precompute_num) {
                    finish = true;
                    break;
                }
            }
        }
    }

    BLOCK_TD.resize(BLOCKS.size());
    for (int i = 0; i < BLOCKS.size(); i++) {
        BLOCK_TD[i] = treedepth_exact(induced(g, BLOCKS[i]));
    }
}
