#pragma once
#include "graph.cpp"

unordered_set<int> open_neighbors(const Graph& g, int v) { return g.adj.at(v); }

unordered_set<int> open_neighbors(const Graph& g, const unordered_set<int>& C) {
    unordered_set<int> ret;
    for (int v : C) {
        for (int a : g.adj.at(v)) {
            if (!C.count(a)) ret.insert(a);
        }
    }
    return ret;
}

unordered_set<int> close_neighbors(const Graph& g, int v) {
    unordered_set<int> ret = open_neighbors(g, v);
    ret.insert(v);
    return ret;
}

unordered_set<int> close_neighbors(const Graph& g, const unordered_set<int>& C) {
    auto ret = open_neighbors(g, C);
    for (int a : C) ret.insert(a);
    return ret;
}

unordered_set<int> intersection(const unordered_set<int>& A, const unordered_set<int>& B) {
    if (A.size() > B.size()) return intersection(B, A);
    unordered_set<int> ret;
    for (int a : A) {
        if (B.count(a)) ret.insert(a);
    }
    return ret;
}

unordered_set<int> join(const unordered_set<int>& A, const unordered_set<int>& B) {
    unordered_set<int> ret(A);
    for (int b : B) ret.insert(b);
    return ret;
}

// A \ B
unordered_set<int> difference(const unordered_set<int>& A, const unordered_set<int>& B) {
    unordered_set<int> ret(A);
    for (int b : B) {
        if (ret.count(b)) ret.erase(b);
    }
    return ret;
}

Graph remove(Graph g, const unordered_set<int>& C) {
    for (int a : C) g.remove_node(a);
    return g;
}

int min_fill_vertex(const Graph& g) {
    int nin = 1e9;
    int ret = -1;
    for (int v : g.nodes) {
        auto X = open_neighbors(g, v);
        int cnt = 0;
        for (int a : X) {
            for (int b : X) {
                if (a < b && !is_adjacent(g, a, b)) {
                    cnt++;
                }
            }
        }
        if (cnt < nin) {
            nin = cnt;
            ret = v;
        }
    }
    assert(ret != -1);
    return ret;
}

int get_min(const unordered_set<int>& C, const unordered_set<int>& min_over) {
    assert(!C.empty());
    if (min_over.empty()) {
        int ret = 1e9;
        for (int a : C) ret = min(ret, a);
        return ret;
    } else {
        int ret = 1e9;
        for (int a : C) {
            if (min_over.count(a)) ret = min(ret, a);
        }
        assert(ret != 1e9);
        return ret;
    }
}

// a \subset b?
bool is_subset(const unordered_set<int>& a, const unordered_set<int>& b) {
    for (int c : a) {
        if (!b.count(c)) return false;
    }
    return true;
}

// sep によって分断されたグラフの連結成分のうち、 v を含むものを返す
unordered_set<int> components_contain(const Graph& g, const unordered_set<int>& sep, int v) {
    assert(!sep.count(v));
    unordered_set<int> ret;
    auto dfs = [&](auto& dfs, int a) -> void {
        ret.insert(a);
        for (int b : g.adj.at(a)) {
            if (!ret.count(b) && !sep.count(b)) {
                dfs(dfs, b);
            }
        }
    };
    dfs(dfs, v);
    return ret;
}

unordered_set<int> compute_A_(const Graph& g, const unordered_set<int>& A, int b, int v) {
    unordered_set<int> C(A);
    C.insert(v);
    auto rem = close_neighbors(g, C);
    if (rem.count(b)) return {};
    auto N = open_neighbors(g, components_contain(g, rem, b));
    return components_contain(g, N, v);
}

vector<unordered_set<int>> unique(const vector<unordered_set<int>>& cs) {
    vector<unordered_set<int>> ret;
    set<vector<int>> ss;
    for (auto& c : cs) {
        vector<int> s(c.begin(), c.end());
        sort(s.begin(), s.end());
        if (!ss.count(s)) {
            ret.push_back(c);
            ss.insert(s);
        }
    }
    return ret;
}

vector<vector<int>> compute_P(const Graph& g, vector<vector<int>> P, unordered_set<int> forbidden) {
    int n = P.size();
    for (auto& a : P) forbidden.insert(a[0]);
    while (1) {
        bool update = false;
        for (int i = 0; i < n; i++) {
            int e = P[i].back();
            for (int a : g.adj.at(e)) {
                if (!forbidden.count(a)) {
                    P[i].push_back(a);
                    forbidden.insert(a);
                    update = true;
                    break;
                }
            }
        }
        if (!update) break;
    }
    return P;
}

constexpr int mod = 1e9 + 7;
using hash_t = int;

hash_t get_hash(vector<int> v, int base) {
    sort(v.begin(), v.end());
    hash_t ret = 0;
    int b = 1;
    for (int a : v) {
        ret += 1LL * b * a % mod;
        if (ret >= mod) ret -= mod;
        b = 1LL * b * base % mod;
    }
    return ret;
}

hash_t get_hash(const unordered_set<int>& A, int base) { return get_hash(vector<int>(A.begin(), A.end()), base); }

hash_t get_hash(const Graph& g) {
    vector<int> es;
    for (int a : g.nodes) {
        for (int b : g.adj.at(a)) {
            if (a < b) es.push_back(629 * a + b);
        }
    }
    return get_hash(g.nodes, 1333) ^ get_hash(es, 13333);
}

bool operator==(const Graph& g1, const Graph& g2) {
    if (g1.n != g2.n || g1.m != g2.m) return false;
    if (g1.nodes != g2.nodes) return false;
    for (int a : g1.nodes) {
        if (!g2.nodes.count(a)) return false;
        if (g1.adj.at(a) != g2.adj.at(a)) return false;
    }
    return true;
}

constexpr int r_prune = 10;
vector<unordered_set<int>> enum_rec(const Graph& g, int k, int a, int b, const unordered_set<int>& A,
                                    const unordered_set<int>& F, unordered_set<int> min_over = {}) {
    // min_over = {} は、全体集合の代わり（毎回全体集合を指定すると重いので）
    if (F.size() > k) return {};
    if (a != get_min(A, min_over)) return {};
    auto Na = open_neighbors(g, A);
    if (F.size() == k && Na != F) return {};
    unordered_set<int> Cb = components_contain(g, close_neighbors(g, A), b);
    // F.size() == k は論文の誤植？スライドは F.size() <= k になっている
    if (A.size() > Cb.size()) return {};
    if (Na.size() > k && A.size() + (Na.size() - k) > min((int)Cb.size(), (g.n - k) / 2)) return {};
    if (!is_subset(F, open_neighbors(g, Cb))) return {};
    if (F.size() <= k && Na == F && open_neighbors(g, Cb) == F && (A.size() <= Cb.size())) {
        return {Na};
    }

    auto dif = difference(Na, F);
    assert(!dif.empty());

    // Lemma 4 による枝刈り
    if (Na.size() > k && A.size() + r_prune * (dif.size() - (k - F.size())) >= (g.n - k) / 2) {
        vector<vector<int>> orig_P;
        for (auto& p : dif) orig_P.push_back({p});
        auto P = compute_P(g, orig_P, close_neighbors(g, A));
        int d = 0;
        sort(P.begin(), P.end(), [](auto& p, auto& q) { return p.size() < q.size(); });
        for (int i = 0; i < P.size() - (k - F.size()); i++) d += P[i].size();
        if (A.size() + d > min((int)Cb.size(), (g.n - k) / 2)) return {};
    }

    vector<unordered_set<int>> ret;
    int v = *dif.begin();
    auto F_(F);
    F_.insert(v);
    for (auto& s : enum_rec(g, k, a, b, A, F_, min_over)) {
        ret.push_back(s);
    }
    auto A_ = compute_A_(g, A, b, v);
    if (!A_.empty()) {
        for (auto& s : enum_rec(g, k, a, b, A_, F, min_over)) {
            ret.push_back(s);
        }
    }
    return unique(ret);
}

Graph local(const Graph& g, const unordered_set<int>& C) {
    Graph ret = induced(g, C);
    for (auto& D : components(remove(g, C))) {
        auto N = open_neighbors(g, D.nodes);
        for (int a : N) {
            for (int b : N) {
                assert(C.count(a) && C.count(b));
                if (a < b) {
                    ret.add_edge(a, b);
                }
            }
        }
    }
    return ret;
}

vector<unordered_set<int>> list_exact_slow(const Graph& g, int k) {
    vector<unordered_set<int>> ret;
    for (int a : g.nodes) {
        for (int b : g.nodes) {
            if (a == b) continue;
            // a と b の順序は重要
            if (!is_adjacent(g, a, b)) {
                unordered_set<int> A = {a}, F = intersection(open_neighbors(g, a), open_neighbors(g, b));
                for (auto& s : enum_rec(g, k, a, b, A, F)) {
                    ret.push_back(s);
                }
            }
        }
    }
    return unique(ret);
}

bool is_separator(const Graph& g, const unordered_set<int>& sep) { return components(remove(g, sep)).size() > 1; }

bool is_separators(const Graph& g, const vector<unordered_set<int>>& seps) {
    for (auto& s : seps) {
        if (!is_separator(g, s)) return false;
    }
    return true;
}

// list all minimal separators of size at most k
vector<unordered_set<int>> list_exact(const Graph& g, int k) {
    vector<unordered_set<int>> ret;
    int v = min_fill_vertex(g);
    unordered_set<int> X = open_neighbors(g, v);
    for (int a : X) {
        for (int b : X) {
            if (a == b) continue;
            if (!is_adjacent(g, a, b)) {
                unordered_set<int> A = {a}, F = intersection(open_neighbors(g, a), open_neighbors(g, b));
                for (auto& s : enum_rec(g, k, a, b, A, F, X)) {
                    ret.push_back(s);
                }
            }
        }
    }
    for (auto& conn : components(remove(g, X))) {
        if (conn.nodes.count(v)) continue;
        auto N = open_neighbors(g, conn.nodes);
        if (N.size() <= k && is_separator(g, N)) ret.push_back(N);
        for (auto& s : list_exact(local(g, join(conn.nodes, X)), k)) {
            ret.push_back(s);
        }
    }
    return unique(ret);
}
