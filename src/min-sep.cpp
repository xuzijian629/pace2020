/*
minimal separator 列挙による解法
*/

#include "lib/lower_bound.cpp"
#include "lib/minimal_separator.cpp"
#include "lib/tw.cpp"

using namespace std;

// treedepth k 以下の分解を作ることができるか
// 不可能なときは empty なグラフを返す

unordered_map<hash_t, Graph> main_memo;
unordered_map<hash_t, tuple<Graph, int>> main_arg_memo;

Graph solve(const Graph& g, int k) {
    // separator が存在しない <=> g が完全グラフ
    if (g.n * (g.n - 1) / 2 == g.m) {
        if (g.n > k) return Graph();
        Graph decomp;
        vector<int> nodes(g.nodes.begin(), g.nodes.end());
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }
        return decomp;
    }

    hash_t h = get_hash(g) ^ k;
    if (main_arg_memo.count(h) && main_arg_memo[h] == make_tuple(g, k)) return main_memo[h];
    if (terminate_by_lb(g, k)) return main_memo[h] = Graph();

    // separator によって 分解される各連結成分の td は 1 以上
    // サイズ k - 1 までの separator を列挙すればいい
    auto seps = list_exact(g, min(k - 1, treewidth_lb(g) + 1));
    if (seps.empty()) return main_memo[h] = Graph();
    for (auto& s : seps) {
        Graph decomp;
        vector<int> nodes(s.begin(), s.end());
        decomp.add_node(nodes[0]);
        decomp.root = nodes[0];
        for (int i = 1; i < nodes.size(); i++) {
            decomp.add_edge(nodes[i - 1], nodes[i]);
        }

        vector<Graph> subtrees;
        bool ok = true;
        auto comps = components(remove(g, s));
        // 頂点数が大きいものほど失敗しやすそう（しかし、頂点数が小さいもので失敗するものをすぐに発見したほうがよさそう）
        sort(comps.begin(), comps.end(), [](auto& a, auto& b) { return a.n < b.n; });
        // TODO: すべての連結成分について、 terminate_by_lb をチェックしたほうがよさそう
        for (auto& C : comps) {
            Graph subtree = solve(C, k - s.size());
            if (!subtree.n) {
                ok = false;
                break;
            }
            subtrees.push_back(subtree);
        }
        if (!ok) continue;
        for (auto& subtree : subtrees) {
            merge(decomp, subtree, nodes.back(), subtree.root);
        }
        return main_memo[h] = decomp;
    }
    return main_memo[h] = Graph();
}

int main() {
    Graph g = read_input();
    for (int k = 1;; k++) {
        Graph decomp = solve(g, k);
        if (decomp.n) {
            print_decomp(decomp);
            break;
        }
    }
}
