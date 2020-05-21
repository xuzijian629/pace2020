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

Graph solve(const Graph& g, int k) {
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
        main_memo[g.nodes] = {0, INT_MAX, nullptr};
        main_memo_ptr = &(main_memo[g.nodes]);
    }

    auto heuristic_decomp = treedepth_heuristic(g);
    int heur_depth = depth(heuristic_decomp, heuristic_decomp.root);
    if (heur_depth <= k) {
        main_memo_ptr->ub = min(main_memo_ptr->ub, heur_depth);
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
                Graph subtree = solve(induced(g, C), k - 1);
                if (!subtree.n()) {
                    main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
                    return Graph();
                }
                merge(decomp, subtree, v, subtree.root);
            }
            main_memo_ptr->ub = min(main_memo_ptr->ub, k);
            main_memo_ptr->ans = new Graph();
            *(main_memo_ptr->ans) = decomp;
            return *(main_memo_ptr->ans);
        }
    }

    // separator によって 分解される各連結成分の td は 1 以上
    // サイズ k - 1 までの separator を列挙すればいい
    auto seps = list_exact(g, min(k - 1, treewidth_ub(g) + 1));
    if (seps.empty()) {
        main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
        return Graph();
    }

    for (auto& s : seps) {
        bool ok = true;
        for (int i = 0; i < BLOCKS.size(); i++) {
            if (intersection(s, BLOCKS[i]).none() && s.count() + BLOCK_TD[i] > k) {
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
            Graph subtree = solve(induced(g, C), k - s.count());
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
        main_memo_ptr->ans = new Graph();
        *(main_memo_ptr->ans) = decomp;
        return *(main_memo_ptr->ans);
    }
    main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
    return Graph();
}

Graph treedepth_decomp(Graph g) {
    // auto ord = find_good_order(g);
    // g = reorder(g, ord.first);
    for (int k = 1;; k++) {
        if (prune_by_td_lb(g, k)) continue;
        Graph decomp = solve(g, k);
        if (decomp.n()) {
            // decomp = reorder(decomp, ord.second);
            return decomp;
        }
    }
}

int main() {
    Graph g = read_input();
    auto start = chrono::steady_clock::now();
    if (g.n() > max_n) init_blocks(g);
    auto finish = chrono::steady_clock::now();
    cerr << "init finished with " << BLOCKS.size() << " blocks ("
         << chrono::duration_cast<chrono::milliseconds>(finish - start).count() << " msec)" << endl;
    print_decomp(treedepth_decomp(g));
}
