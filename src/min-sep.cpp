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
    BITSET* sep = nullptr;  // either one of tree or sep
    main_memo_t() {
        // entry とは別にまた sizeof(BITSET) 分あるらしい
        sep_dictionary.change_memcapacity((sizeof(BITSET) << 1) + sizeof(main_memo_t), Sep_Dictionary::op_t::SUB);
    }
    ~main_memo_t() { delete this->sep; }
    void register_sep(const BITSET& sep) {
        this->erase_record();
        sep_dictionary.change_memcapacity(sizeof(BITSET), Sep_Dictionary::op_t::SUB);
        this->sep = new BITSET(sep);
    }
    void register_tree(const Graph& tree) {
        this->erase_record();
        sep_dictionary.change_memcapacity(sizeof(Graph), Sep_Dictionary::op_t::SUB);
        this->tree = new Graph(tree);
    }

private:
    void erase_record() {
        if (this->sep != nullptr) {
            sep_dictionary.change_memcapacity(sizeof(BITSET), Sep_Dictionary::op_t::ADD);
            delete this->sep;
            this->sep = nullptr;
        }
        if (this->tree != nullptr) {
            sep_dictionary.change_memcapacity(sizeof(Graph), Sep_Dictionary::op_t::ADD);
            delete this->tree;
            this->tree = nullptr;
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
    assert(!(main_memo_ptr->tree == nullptr && main_memo_ptr->sep == nullptr));
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
    assert(false);
}

// return true if there is an answer, either the separator or the tree is guaranteed to be registered
// if you want to get the graph, call get_tree_from_main_memo(g)
bool solve(const Graph& g, int k, int use_block_size_max = 1e9) {
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
    main_memo.clear();
    sep_dictionary = Sep_Dictionary();
    print_decomp(treedepth_decomp(g));
}
