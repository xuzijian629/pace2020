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
struct main_memo_t {
    int lb = 0;
    int ub = INT_MAX;
    Graph* ans = nullptr;
};
unordered_map<BITSET, main_memo_t> main_memo;

Graph solve(const Graph& g, int k, bool use_block) {
    if (g.n() == 1) {
        return Graph(g.nodes._Find_first());
    }

    main_memo_t* main_memo_ptr;
    // look up memo,
    // if k < non trivial lb, return false
    // if k >= ub, return memorized ans
    if (main_memo.count(g.nodes)) {
        main_memo_ptr = &(main_memo[g.nodes]);
        if (k < main_memo_ptr->lb) {
            return Graph();
        }
        if (main_memo_ptr->ub <= k) {
            return *(main_memo_ptr->ans);
        }
    } else {
        sep_dictionary.reduce_memcapacity(_MY_BITSET_MEMBYTES + 8);
        main_memo[g.nodes] = {0, INT_MAX, nullptr};
        main_memo_ptr = &(main_memo[g.nodes]);
    }

    auto heuristic_decomp = treedepth_heuristic(g);
    int heur_depth = depth(heuristic_decomp, heuristic_decomp.root);
    if (heur_depth <= k) {
        main_memo_ptr->ub = min(main_memo_ptr->ub, heur_depth);
        if (main_memo_ptr->ans == nullptr) {
            sep_dictionary.reduce_memcapacity(_ADJ_MEMBYTES);
        } else {
            delete main_memo_ptr->ans;
        }
        main_memo_ptr->ans = new Graph();
        *(main_memo_ptr->ans) = heuristic_decomp;
        return *(main_memo_ptr->ans);
    }

    // apex vertex があれば選ぶ
    FOR_EACH(v, g.nodes) {
        if (at(g.adj, v).count() == g.n() - 1) {
            Graph decomp(v);
            BITSET rem;
            rem.set(v);
            for (auto& C : components(remove(g, rem))) {
                Graph subtree = solve(induced(g, C), k - 1, use_block);
                if (!subtree.n()) {
                    main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
                    return Graph();
                }
                merge(decomp, subtree, v, subtree.root);
            }
            main_memo_ptr->ub = min(main_memo_ptr->ub, k);
            if (main_memo_ptr->ans == nullptr) {
                sep_dictionary.reduce_memcapacity(_ADJ_MEMBYTES);
            } else {
                delete main_memo_ptr->ans;
            }
            main_memo_ptr->ans = new Graph();
            *(main_memo_ptr->ans) = decomp;
            return *(main_memo_ptr->ans);
        }
    }

    // separator によって 分解される各連結成分の td は 1 以上
    // サイズ k - 1 までの separator を列挙すればいい
    auto seps = list_exact(g, min(k - 1, treewidth_ub(g)));
    if (seps.empty()) {
        main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
        return Graph();
    }

    for (auto& s : seps) {
        bool ok = true;
        int n = g.n();
        if (use_block && min_block_size != 1e9) {
            int lim = k - s.count();
            for (int i = min_block_size; i != -1; i = NEXT_BLOCK[i]) {
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

        Graph decomp;
        vector<int> nodes;
        FOR_EACH(v, s) nodes.push_back(v);
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }

        vector<Graph> subtrees;
        for (auto& C : comps) {
            Graph subtree = solve(induced(g, C), k - s.count(), use_block);
            if (!subtree.n()) {
                ok = false;
                break;
            }
            subtrees.push_back(subtree);
        }
        if (!ok) continue;
        for (auto& subtree : subtrees) {
            merge(decomp, subtree, nodes.back(), subtree.root);
        }
        main_memo_ptr->ub = min(main_memo_ptr->ub, k);
        if (main_memo_ptr->ans == nullptr) {
            sep_dictionary.reduce_memcapacity(_ADJ_MEMBYTES);
        } else {
            delete main_memo_ptr->ans;
        }
        main_memo_ptr->ans = new Graph();
        *(main_memo_ptr->ans) = decomp;
        return *(main_memo_ptr->ans);
    }
    main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
    return Graph();
}

Graph treedepth_decomp(Graph g, bool use_block = true) {
    for (int k = 1;; k++) {
        if (prune_by_td_lb(g, k)) continue;
        Graph decomp = solve(g, k, use_block);
        if (decomp.n()) {
            return decomp;
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
    print_decomp(treedepth_decomp(g));
}
