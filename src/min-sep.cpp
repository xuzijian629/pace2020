/*
minimal separator 列挙による解法
*/

#include "lib/balanced_separator.cpp"
#include "lib/lower_bound.cpp"
#include "lib/minimal_separator.cpp"
#include "lib/precompute.cpp"
#include "lib/reorder.cpp"
#include "lib/tw.cpp"

using namespace std;

// treedepth k 以下の分解を作ることができるか
// 不可能なときは empty なグラフを返す

// induced subgraph なので g 覚えなくて良い
class main_memo_t {
public:
    int lb = 0;
    int ub = INT_MAX;
    Graph* tree = nullptr;
    BITSET* sep = nullptr;
    BITSET* simplicial = nullptr;  // either one of tree or sep or simplicial
    main_memo_t() {}
    ~main_memo_t() { this->erase_record(); }
    void register_sep(const BITSET& sep) {
        this->erase_record();
        this->sep = new BITSET(sep);
    }
    void register_tree(const Graph& tree) {
        this->erase_record();
        this->tree = new Graph(tree);
    }
    void register_simplicial(const BITSET& simplicial) {
        this->erase_record();
        this->simplicial = new BITSET(simplicial);
    }

private:
    void erase_record() {
        if (this->sep != nullptr) {
            delete this->sep;
            this->sep = nullptr;
        }
        if (this->tree != nullptr) {
            delete this->tree;
            this->tree = nullptr;
        }
        if (this->simplicial != nullptr) {
            delete this->simplicial;
            this->simplicial = nullptr;
        }
    }
};

unordered_map<BITSET, main_memo_t> main_memo;

// get tree from main memo
Graph get_tree_from_main_memo(const Graph& g) {
    if (g.nodes.count() == 1) {
        return Graph(g.nodes._Find_first());
    }
    main_memo_t* main_memo_ptr;
    assert(main_memo.count(g.nodes));
    main_memo_ptr = &(main_memo[g.nodes]);
    assert(!(main_memo_ptr->tree == nullptr && main_memo_ptr->sep == nullptr && main_memo_ptr->simplicial == nullptr));
    if (main_memo_ptr->tree != nullptr) {
        return *(main_memo_ptr->tree);
    }
    if (main_memo_ptr->sep != nullptr) {
        assert(main_memo_ptr->sep->count());
        size_t v = main_memo_ptr->sep->_Find_first();
        Graph decomp(v);
        size_t pre_v = v;
        for (v = main_memo_ptr->sep->_Find_next(v); v < main_memo_ptr->sep->size();
             v = main_memo_ptr->sep->_Find_next(v)) {
            decomp.add_edge(pre_v, v);
            pre_v = v;
        }
        for (auto& C : components(remove(g, *(main_memo_ptr->sep)))) {
            Graph subtree = get_tree_from_main_memo(induced(g, C));
            merge(decomp, subtree, pre_v, subtree.root);
        }
        return decomp;
    }
    if (main_memo_ptr->simplicial != nullptr) {
        assert(main_memo_ptr->simplicial->count());
        Graph decomp = get_tree_from_main_memo(induced(g, *(main_memo_ptr->simplicial)));
        // for each v in simplicial nodes
        for (auto v = main_memo_ptr->simplicial->_Find_first(); v < main_memo_ptr->simplicial->size();
             v = main_memo_ptr->simplicial->_Find_next(v)) {
            // for each w adjacent to v, take target among those w s.t. placed in the deepest
            // since the adjacent vertices form a clique, they are on a path between the root to the deepst one
            // thus, just take the last node as the "target" in dfs order that is adjacent to v
            auto dfs = [&](auto& dfs, int w, int p, int& target) -> void {
                if (g.adj[v][w]) target = w;
                FOR_EACH(s, at(decomp.adj, v)) {
                    if (s != p) {
                        dfs(dfs, s, v, target);
                    }
                }
            };
            int target = -1;
            dfs(dfs, g.root, -1, target);
            assert(target >= 0);
            decomp.add_edge(target, v);
        }
        return decomp;
    }
    assert(false);
}

Graph remove_simplicial(const Graph& g, BITSET& removed) {
    Graph h(g);
    array<int, BITSET_MAX_SIZE> deg;
    queue<int> que;
    FOR_EACH(v, g.nodes) {
        deg[v] = at(g.adj, v).count();
        que.push(v);
    }
    vector<int> simplicials;
    while (!que.empty()) {
        int v = que.front();
        que.pop();
        if (!h.nodes.test(v)) continue;
        bool ok = true;
        FOR_EACH(u, at(h.adj, v)) {
            if (deg[u] <= k) {
                ok = false;
                break;
            }
            FOR_EACH(w, at(h.adj, v)) {
                if (u < w && !at(h.adj, u).test(w)) {
                    ok = false;
                    break;
                }
            }
            if (!ok) break;
        }
        if (ok) {
            FOR_EACH(u, at(h.adj, v)) {
                que.push(u);
                deg[u]--;
                assert(deg[u] >= 0);
            }
            h.remove_node(v);
            simplicials.push_back(v);
        }
    }
    return h;
}

// return true if there is an answer, either the separator or the tree is guaranteed to be registered
// if you want to get the graph, call get_tree_from_main_memo(g)
bool solve(const Graph& g, int k, int use_block_size_max = 1e9, bool skip_simplicial_rule = false) {
    assert(k >= 1);
    if (g.n() == 1) return true;

    main_memo_t* main_memo_ptr;
    // look up memo,
    // if k < non trivial lb, return false
    // if k >= ub, return memorized ans
    if (main_memo.count(g.nodes)) {
        main_memo_ptr = &(main_memo[g.nodes]);
        if (k < main_memo_ptr->lb) {
            return false;
        }
        if (main_memo_ptr->ub <= k) {
            return true;
        }
    } else {
        main_memo_ptr = &(main_memo[g.nodes]);
    }

    // simplicial rule
    if (!skip_simplicial_rule) {
        BITSET simplicial = 0;
        Graph next_g;
        next_g = remove_simplicial(g, simplicial);
        if (simplicial.any()) {
            bool ok = solve(next_g, k, use_block_size_max, true);
            if (ok) {
                main_memo_ptr->ub = min(main_memo_ptr->ub, k);
                main_memo_ptr->register_simplicial(simplicial);
                return true;
            } else {
                main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
                main_memo_ptr->register_simplicial(simplicial);
                return false;
            }
        }
    }

    auto heuristic_decomp = treedepth_heuristic(g);
    int heur_depth = depth(heuristic_decomp, heuristic_decomp.root);
    if (heur_depth <= k) {
        main_memo_ptr->ub = min(main_memo_ptr->ub, heur_depth);
        main_memo_ptr->register_tree(heuristic_decomp);
        return true;
    }

    // apex vertex があれば選ぶ
    FOR_EACH(v, g.nodes) {
        if (at(g.adj, v).count() == g.n() - 1) {
            BITSET rem;
            rem.set(v);
            BITSET s = rem;
            for (auto& C : components(remove(g, rem))) {
                bool ok = solve(induced(g, C), k - 1, use_block_size_max);
                if (!ok) {
                    main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
                    return false;
                }
            }
            main_memo_ptr->ub = min(main_memo_ptr->ub, k);
            main_memo_ptr->register_sep(s);
            return true;
        }
    }

    // separator によって 分解される各連結成分の td は 1 以上
    // サイズ k - 1 までの separator を列挙すればいい
    auto seps = list_exact(g, min(k - 1, treewidth_ub(g)));
    if (seps.empty()) {
        main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
        return false;
    }

    for (auto& s : seps) {
        bool ok = true;
        int n = g.n();
        if (min_block_size != 1e9) {
            int lim = k - s.count();
            for (int i = min_block_size; i != -1 && i <= use_block_size_max; i = NEXT_BLOCK[i]) {
                if (i < n - s.count()) {
                    for (int j = 0; j < BLOCKS[i].size(); j++) {
                        if (BLOCK_TD[i][j] <= lim) break;
                        if (is_subset(BLOCKS[i][j], g.nodes) && intersection(BLOCKS[i][j], s).none()) {
                            ok = false;
                            break;
                        }
                    }
                    if (!ok) break;
                }
                if (i <= n) {
                    for (int j = 0; j < BLOCKS[i].size(); j++) {
                        if (BLOCK_TD[i][j] <= k) break;
                        if (is_subset(join(BLOCKS[i][j], s), g.nodes)) {
                            ok = false;
                            break;
                        }
                    }
                    if (!ok) break;
                }
            }
        }
        if (!ok) continue;

        auto comps = components(remove(g, s));
        // 頂点数が大きいものほど失敗しやすそう（しかし、頂点数が小さいもので失敗するものをすぐに発見したほうがよさそう）
        sort(comps.begin(), comps.end(), [](auto& a, auto& b) { return a.count() < b.count(); });
        // 先に treewidth lb による枝刈りをしておく
        for (auto& C : comps) {
            int lim = k - s.count();
            if (prune_by_td_lb(induced(g, C), lim)) {
                ok = false;
                break;
            }
        }
        if (!ok) continue;
        for (auto& C : comps) {
            ok = solve(induced(g, C), k - s.count(), use_block_size_max);
            if (!ok) break;
        }
        if (!ok) continue;
        main_memo_ptr->ub = min(main_memo_ptr->ub, k);
        main_memo_ptr->register_sep(s);
        return true;
    }
    main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
    return false;
}

Graph treedepth_decomp(Graph g, int use_block_max_size = 1e9) {
    for (int k = 1;; k++) {
        if (prune_by_td_lb(g, k)) continue;
        if (solve(g, k, use_block_max_size)) {
            return get_tree_from_main_memo(g);
        }
    }
}

int main() {
    Graph g = read_input();
    auto start = chrono::steady_clock::now();
    init_blocks(g, tl_preprocess);
    auto finish = chrono::steady_clock::now();
    cerr << "init finished: " << chrono::duration_cast<chrono::milliseconds>(finish - start).count() << " [msec]"
         << endl;
    // main_memo.clear();
    // sep_dictionary = Sep_Dictionary();
    print_decomp(treedepth_decomp(g));
}
