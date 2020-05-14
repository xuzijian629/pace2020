/*
minimal separator 列挙による解法
*/

#include "lib/balanced_separator.cpp"
#include "lib/lower_bound.cpp"
#include "lib/minimal_separator.cpp"
#include "lib/tw.cpp"

using namespace std;

// treedepth k 以下の分解を作ることができるか
// 不可能なときは empty なグラフを返す

struct main_memo_t {
    Graph g;
    int lb = 0;
    int ub = INT_MAX;
    Graph* ans = nullptr;
};
unordered_map<BITSET, main_memo_t> main_memo;

Graph solve(const Graph& g, int k) {
    // separator が存在しない <=> g が完全グラフ
    int n = g.n();
    if (n * (n - 1) / 2 == g.m()) {
        if (n > k) {
            return Graph();
        }
        Graph decomp;
        vector<int> nodes;
        FOR_EACH(v, g.nodes) nodes.push_back(v);
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }
        return decomp;
    }

    auto heuristic_decomp = treedepth_heuristic(g);
    if (depth(heuristic_decomp, heuristic_decomp.root) <= k) return heuristic_decomp;

    main_memo_t* main_memo_ptr;
    // look up memo,
    // if k < non trivial lb, return false
    // if k >= ub, return memorized ans
    if (main_memo.count(g.nodes) && main_memo[g.nodes].g == g) {
        main_memo_ptr = &(main_memo[g.nodes]);
        if (k < main_memo_ptr->lb) {
            return Graph();
        }
        if (main_memo_ptr->ub <= k) {
            return *(main_memo_ptr->ans);
        }
    } else {
        main_memo[g.nodes] = {g, 0, INT_MAX, nullptr};
        main_memo_ptr = &(main_memo[g.nodes]);
    }

    // separator によって 分解される各連結成分の td は 1 以上
    // サイズ k - 1 までの separator を列挙すればいい
    auto seps = list_exact(g, min(k - 1, treewidth_ub(g) + 1));
    if (seps.empty()) {
        main_memo_ptr->lb = max(main_memo_ptr->lb, k + 1);
        return Graph();
    }

    for (auto& s : seps) {
        Graph decomp;
        vector<int> nodes;
        FOR_EACH(v, s) nodes.push_back(v);
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }

        vector<Graph> subtrees;
        bool ok = true;
        auto comps = components(remove(g, s));
        // 頂点数が大きいものほど失敗しやすそう（しかし、頂点数が小さいもので失敗するものをすぐに発見したほうがよさそう）
        sort(comps.begin(), comps.end(), [](auto& a, auto& b) { return a.count() < b.count(); });

        // 先に treewidth lb による枝刈りをしておく
        for (auto& C : comps) {
            int lim = k - s.count();
            if (treedepth_lb(induced(g, C), lim) > lim) {
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

int main() {
    Graph g = read_input();
    for (int k = 1;; k++) {
        if (treedepth_lb(g, k) > k) continue;
        Graph decomp = solve(g, k);
        if (decomp.n()) {
            print_decomp(decomp);
            break;
        }
    }
}
