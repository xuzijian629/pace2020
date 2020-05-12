#pragma once
#include "graph.cpp"

inline BITSET open_neighbors(const Graph& g, int v) { return at(g.adj, v); }

BITSET open_neighbors(const Graph& g, const BITSET& C) {
    BITSET ret;
    FOR_EACH(v, C) { ret |= at(g.adj, v); }
    return ret ^ (ret & C);
}

BITSET close_neighbors(const Graph& g, int v) {
    auto ret = open_neighbors(g, v);
    ret.set(v);
    return ret;
}

BITSET close_neighbors(const Graph& g, const BITSET& C) {
    BITSET ret = C;
    FOR_EACH(v, C) { ret |= at(g.adj, v); }
    return ret;
}

inline BITSET intersection(const BITSET& A, const BITSET& B) { return A & B; }

inline BITSET join(const BITSET& A, const BITSET& B) { return A | B; }

// A \ B
inline BITSET difference(const BITSET& A, const BITSET& B) { return A ^ (A & B); }

Graph remove(Graph g, const BITSET& C) { return induced(g, difference(g.nodes, C)); }

int min_fill_vertex(const Graph& g) {
    int nin = 1e9;
    int ret = -1;
    FOR_EACH(v, g.nodes) {
        auto X = open_neighbors(g, v);
        int cnt = 0;
        FOR_EACH(a, X) { cnt += difference(X, at(g.adj, a)).count(); }
        if (cnt < nin) {
            nin = cnt;
            ret = v;
        }
    }
    assert(ret != -1);
    return ret;
}

inline int get_min(const BITSET& C) { return C._Find_first(); }

inline int get_min(const BITSET& C, BITSET* min_over) {
    if (min_over) return get_min(C & *min_over);
    return get_min(C);
}

// A \subset B?
inline bool is_subset(const BITSET& A, const BITSET& B) { return (A & B) == A; }

// sep によって分断されたグラフの連結成分のうち、 v を含むものを返す
BITSET components_contain(const Graph& g, const BITSET& sep, int v) {
    assert(!sep.test(v));
    BITSET ret;
    auto bfs = [&](auto& bfs, const BITSET& cur) -> void {
        BITSET nxt;
        FOR_EACH(v, cur) { nxt |= difference(at(g.adj, v), join(ret, sep)); }
        if (nxt.any()) {
            ret |= nxt;
            bfs(bfs, nxt);
        }
    };
    BITSET cur;
    ret.set(v);
    cur.set(v);
    bfs(bfs, cur);
    return ret;
}

BITSET compute_A_(const Graph& g, const BITSET& A, int b, int v) {
    BITSET C(A);
    C.set(v);
    auto rem = close_neighbors(g, C);
    if (rem.test(b)) return BITSET();
    auto N = open_neighbors(g, components_contain(g, rem, b));
    return components_contain(g, N, v);
}

// for std::bitset
namespace std {
bool operator<(const BITSET& a, const BITSET& b) {
    int k = (a ^ b)._Find_first();
    if (k == BITSET_MAX_SIZE) return false;
    return a[k] < b[k];
}
}  // namespace std

vector<BITSET> unique(vector<BITSET> cs) {
    sort(cs.begin(), cs.end());
    cs.erase(unique(cs.begin(), cs.end()), cs.end());
    sort(cs.begin(), cs.end(), [](auto& p, auto& q) { return p.count() < q.count(); });
    return cs;
}

vector<vector<int>> compute_P(const Graph& g, vector<vector<int>> P, BITSET forbidden) {
    int n = P.size();
    for (auto& a : P) forbidden.set(a[0]);
    while (1) {
        bool update = false;
        for (int i = 0; i < n; i++) {
            int e = P[i].back();
            auto dif = difference(at(g.adj, e), forbidden);
            int a = dif._Find_first();
            if (a < dif.size()) {
                P[i].push_back(a);
                forbidden.set(a);
                update = true;
                break;
            }
        }
        if (!update) break;
    }
    return P;
}

constexpr int mod = 1e9 + 7;
using hash_t = int;

struct Init {
    array<int, BITSET_MAX_SIZE> pows;
    Init() {
        pows[0] = 1;
        for (int i = 1; i < BITSET_MAX_SIZE; i++) {
            pows[i] = (long long)pows[i - 1] * 1333 % mod;
        }
    }
} init;

hash_t get_hash(const BITSET& v) {
    hash_t ret = 0;
    FOR_EACH(a, v) {
        ret += init.pows[a];
        if (ret >= mod) ret -= mod;
    }
    return ret;
}

hash_t get_hash(const Graph& g) {
    hash_t ret = get_hash(g.nodes);
    FOR_EACH(a, g.nodes) { ret ^= get_hash(at(g.adj, a)); }
    return ret;
}

bool operator==(const Graph& g1, const Graph& g2) {
    if (g1.nodes != g2.nodes) return false;
    FOR_EACH(a, g1.nodes) {
        if (at(g1.adj, a) != at(g2.adj, a)) return false;
    }
    return true;
}

constexpr int r_prune = 10;
vector<BITSET> enum_rec(const Graph& g, int k, int a, int b, const BITSET& A, const BITSET& F, BITSET* min_over) {
    // min_over = nullptr は、全体集合の代わり（毎回全体集合を指定すると重いので）
    if (F.count() > k) return {};
    if (a != get_min(A, min_over)) return {};
    auto Na = open_neighbors(g, A);
    BITSET Cb = components_contain(g, close_neighbors(g, A), b);
    if (A.count() > Cb.count()) return {};
    if (!is_subset(F, open_neighbors(g, Cb))) return {};
    if (F.count() <= k && Na == F) {
        assert(open_neighbors(g, Cb) == F);
        if (b == get_min(Cb, min_over))
            return {Na};
        else
            return {};
    }

    auto dif = difference(Na, F);
    assert(dif.any());

    // Lemma 4 による枝刈り
    if (Na.count() > k && A.count() + r_prune * (dif.count() - (k - F.count())) >= (g.n() - k) / 2) {
        vector<vector<int>> orig_P;
        FOR_EACH(p, dif) orig_P.push_back({p});
        auto P = compute_P(g, orig_P, close_neighbors(g, A));
        int d = 0;
        sort(P.begin(), P.end(), [](auto& p, auto& q) { return p.size() < q.size(); });
        for (int i = 0; i < P.size() - (k - F.count()); i++) d += P[i].size();
        if (A.count() + d > min((int)Cb.count(), (g.n() - k) / 2)) return {};
    }

    vector<BITSET> ret;
    int v = dif._Find_first();
    auto F_(F);
    F_.set(v);
    for (auto& s : enum_rec(g, k, a, b, A, F_, min_over)) {
        ret.push_back(s);
    }
    auto A_ = compute_A_(g, A, b, v);
    if (A_.any()) {
        for (auto& s : enum_rec(g, k, a, b, A_, F, min_over)) {
            ret.push_back(s);
        }
    }
    return unique(ret);
}

Graph local(const Graph& g, const BITSET& C) {
    Graph ret = induced(g, C);
    for (auto& D : components(remove(g, C))) {
        auto N = open_neighbors(g, D);
        assert(is_subset(N, C));
        FOR_EACH(a, N) {
            ret.adj[a] |= N;
            ret.adj[a].reset(a);
        }
    }
    return ret;
}

struct sep_memo_t {
    int k;
    vector<BITSET> seps;
};
unordered_map<BITSET, sep_memo_t> sep_memo;

vector<BITSET> list_exact_slow(const Graph& g, int k) {
    vector<BITSET> ret;
    FOR_EACH(a, g.nodes) {
        FOR_EACH(b, g.nodes) {
            if (a == b) continue;
            // a と b の順序は重要
            if (!is_adjacent(g, a, b)) {
                BITSET A, F = intersection(open_neighbors(g, a), open_neighbors(g, b)), *min_over = nullptr;
                A.set(a);
                for (auto& s : enum_rec(g, k, a, b, A, F, min_over)) {
                    ret.push_back(s);
                }
            }
        }
    }
    return unique(ret);
}

bool is_separator(const Graph& g, const BITSET& sep) { return components(remove(g, sep)).size() > 1; }

bool is_separators(const Graph& g, const vector<BITSET>& seps) {
    for (auto& s : seps) {
        if (!is_separator(g, s)) return false;
    }
    return true;
}

// list all minimal separators of size at most k
vector<BITSET> list_exact(const Graph& g, int k) {
    vector<BITSET> ret;
    if (sep_memo.count(g.nodes)) {
        if (k <= sep_memo[g.nodes].k) {
            for (const auto &sep : sep_memo[g.nodes].seps) {
                if (sep.count() <= k) ret.push_back(sep);
                else break;
            }
            return ret;
        }
    }
    int v = min_fill_vertex(g);
    BITSET X = open_neighbors(g, v);
    FOR_EACH(a, X) {
        FOR_EACH(b, X) {
            if (a == b) continue;
            if (!is_adjacent(g, a, b)) {
                BITSET A, F = intersection(open_neighbors(g, a), open_neighbors(g, b));
                A.set(a);
                for (auto& s : enum_rec(g, k, a, b, A, F, &X)) {
                    ret.push_back(s);
                }
            }
        }
    }
    for (auto& conn : components(remove(g, X))) {
        if (conn.test(v)) continue;
        auto N = open_neighbors(g, conn);
        if (N.count() <= k) ret.push_back(N);
        for (auto& s : list_exact(local(g, join(conn, X)), k)) {
            ret.push_back(s);
        }
    }
    ret = unique(ret);
    if (ret.size() > 64) sep_memo[g.nodes] = { k, ret };
    return ret;
}
