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

BITSET compute_A_(const Graph& g, const BITSET& A, int b, int v, BITSET* Cb) {
    BITSET C(A);
    C.set(v);
    auto rem = close_neighbors(g, C);
    if (rem.test(b)) return BITSET();
    *Cb = components_contain(g, rem, b);
    auto N = open_neighbors(g, *Cb);
    return components_contain(g, N, v);
}

// for std::bitset
namespace std {
bool operator<(const BITSET& a, const BITSET& b) {
    int k = (a ^ b)._Find_first();
    if (k == BITSET_ACTUAL_SIZE) return false;
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
    using T = pair<int, int>;
    priority_queue<T, vector<T>, greater<>> que;
    for (int i = 0; i < n; i++) que.emplace(1, i);
    while (1) {
        bool update = false;
        while (!que.empty()) {
            auto top = que.top();
            que.pop();
            int i = top.second;
            int e = P[i].back();
            auto dif = difference(at(g.adj, e), forbidden);
            int a = dif._Find_first();
            if (a < dif.size()) {
                P[i].push_back(a);
                forbidden.set(a);
                update = true;
                que.emplace(top.first + 1, i);
                break;
            }
        }
        if (!update) break;
    }
    return P;
}

constexpr int mod = 1000000007;  // (1 << 29) - 1;
using hash_t = int;

struct Init {
    array<int, BITSET_MAX_SIZE> pows, pows2;
    Init() {
        pows[0] = 1;
        pows2[0] = 1;
        for (int i = 1; i < BITSET_MAX_SIZE; i++) {
            pows[i] = (long long)pows[i - 1] * 1333 % mod;
            pows2[i] = (long long)pows2[i - 1] * 629 % mod;
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
    FOR_EACH(a, g.nodes) {
        ret += (long long)get_hash(at(g.adj, a)) * init.pows2[a] % mod;
        if (ret >= mod) ret -= mod;
    }
    ret++;
    return ret;
}

bool operator==(const Graph& g1, const Graph& g2) {
    if (g1.nodes != g2.nodes) return false;
    FOR_EACH(a, g1.nodes) {
        if (at(g1.adj, a) != at(g2.adj, a)) return false;
    }
    return true;
}

template <class T>
void vector_concat(std::vector<T>& a, std::vector<T>& b) {
    size_t pre_size = a.size();
    a.resize(pre_size + b.size());
    std::copy(b.begin(), b.end(), a.begin() + pre_size);
}

constexpr int r_prune = 5;
void enum_rec(vector<BITSET>& ret, const Graph& g, int k, int a, int b, const BITSET& A, const BITSET& F,
              BITSET* min_over, const BITSET& Cb) {
    // min_over = nullptr は、全体集合の代わり（毎回全体集合を指定すると重いので）
    if (F.count() > k) return;
    auto Na = open_neighbors(g, A);
    if (A.count() > Cb.count()) return;
    if (!is_subset(F, open_neighbors(g, Cb))) return;
    if (Na == F) {
        assert(open_neighbors(g, Cb) == F);
        if (b == get_min(Cb, min_over)) {
            ret.push_back(Na);
            return;
        } else
            return;
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
        if (A.count() + d > min((int)Cb.count(), (g.n() - k) / 2)) return;
    }

    int v = dif._Find_first();
    auto F_(F);
    F_.set(v);
    //
    enum_rec(ret, g, k, a, b, A, F_, min_over, Cb);
    //
    bool can_cut = (min_over ? (*min_over).test(v) && v < a : v < a);
    if (!can_cut) {
        BITSET nxtCb;
        auto A_ = compute_A_(g, A, b, v, &nxtCb);
        if (A_.any()) {
            //
            enum_rec(ret, g, k, a, b, A_, F, min_over, nxtCb);
            //
        }
    }
    return;
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

//====================================================
// format of separator memo
//----------------------------------------------------
// 0      h; (hash number, 1-indexed)
// 1      k; (corresponded to treedepth)
// 2      adjlen; adjsprs expressions length
// 3      seplen; separator expressions length
// 4      nodes; (vertex set)
// 5      { adjsprs expressions }
// 5+adjl { separator expressions }
//----------------------------------------------------
// total: (adjlen + seplen + 5) * sizeof(BITSET) bytes
//====================================================
#ifndef _MY_STATIC_MEMBYTE
#define _MY_STATIC_MEMBYTE 7516192768  // 7GB for seps
#endif
#define _MY_STATIC_MEMSIZE (_MY_STATIC_MEMBYTE / sizeof(BITSET))
std::vector<BITSET> static_memory(_MY_STATIC_MEMSIZE);
size_t static_memory_ptr = 0;

// use as static class
class Sep_Dictionary {
private:
    unordered_map<hash_t, size_t> sep_memo_mp;  // hash is 1-indexed, static_memory[sep_memo_mp[h]] == h
    // return erased length
    size_t erase_hash(const hash_t h) {
        size_t offset = sep_memo_mp[h];
        assert(static_memory[offset].to_ullong() == size_t(h));
        size_t adjlen = static_memory[offset + 2].to_ullong();
        size_t seplen = static_memory[offset + 3].to_ullong();
        size_t totallen = 5 + adjlen + seplen;
        for (size_t i = offset; i < offset + totallen; ++i) {
            static_memory[i].reset();
        }
        sep_memo_mp.erase(h);
        return totallen;
    }
    // return erased length
    size_t erase_static_memory(const size_t offset) {
        hash_t h = static_memory[offset].to_ullong();
        assert(sep_memo_mp.count(h));
        assert(sep_memo_mp[h] == offset);
        return this->erase_hash(h);
    }
    // static_memory_ptr will be automatically set to the offset
    void static_memory_alloc(size_t n) {
        if (static_memory_ptr + n >= static_memory.size()) {
            static_memory_ptr = 0;
        }
        for (size_t i = static_memory_ptr; i < static_memory_ptr + n;) {
            if (static_memory[i].none())
                i++;
            else
                i += erase_static_memory(i);
        }
    }

public:
    ~Sep_Dictionary() {
        vector<hash_t> hs(this->sep_memo_mp.size());
        size_t i = 0;
        for (auto itr = sep_memo_mp.begin(); itr != sep_memo_mp.end(); ++itr) {
            hs[i++] = itr->first;
        }
        for (const auto h : hs) {
            this->erase_hash(h);
        }
    }
    // LRU policy
    void insert(const hash_t h, const int k, const BITSET& nodes, const ADJSPRS& adjsprs, const vector<BITSET>& seps) {
        if (this->sep_memo_mp.count(h)) {
            this->erase_hash(h);
        }
        size_t adjlen = adjsprs.size();
        size_t seplen = seps.size();
        size_t totallen = 5 + adjlen + seplen;
        this->static_memory_alloc(totallen);
        sep_memo_mp[h] = static_memory_ptr;
        assert(h >= 1);
        static_memory[static_memory_ptr++] = h;
        static_memory[static_memory_ptr++] = k;
        static_memory[static_memory_ptr++] = adjlen;
        static_memory[static_memory_ptr++] = seplen;
        static_memory[static_memory_ptr++] = nodes;
        for (size_t i = 0; i < adjlen; ++i) {
            static_memory[static_memory_ptr++] = adjsprs[i];
        }
        for (size_t i = 0; i < seplen; ++i) {
            static_memory[static_memory_ptr++] = seps[i];
        }
    }
    // return offset
    size_t access(const hash_t h) {
        if (sep_memo_mp.count(h)) {
            return sep_memo_mp[h];
        } else {
            return SIZE_MAX;
        }
    }
    bool is_equal(const size_t offset, const BITSET& nodes, const ADJSPRS& adjsprs) {
        if (static_memory[offset + 2].to_ullong() != adjsprs.size()) return false;
        if (static_memory[offset + 4] != nodes) return false;
        for (size_t i = 0; i < adjsprs.size(); ++i) {
            if (static_memory[offset + 5 + i] != adjsprs[i]) return false;
        }
        return true;
    }
    size_t get_k(const size_t offset) { return static_memory[offset + 1].to_ullong(); }
    // return [l, r)
    std::pair<size_t, size_t> get_sep_range(const size_t offset) {
        size_t adjlen = static_memory[offset + 2].to_ullong();
        size_t seplen = static_memory[offset + 3].to_ullong();
        return {offset + 5 + adjlen, offset + 5 + adjlen + seplen};
    }
};

Sep_Dictionary sep_dictionary;

vector<BITSET> list_exact_slow(const Graph& g, int k) {
    vector<BITSET> ret;
    FOR_EACH(a, g.nodes) {
        FOR_EACH(b, g.nodes) {
            if (a == b) continue;
            // a と b の順序は重要
            if (!is_adjacent(g, a, b)) {
                BITSET A, F = intersection(open_neighbors(g, a), open_neighbors(g, b)), *min_over = nullptr;
                auto Na = open_neighbors(g, a);
                auto Cb = components_contain(g, Na, b);
                A = components_contain(g, open_neighbors(g, Cb), a);
                if (get_min(A) != a) continue;
                { enum_rec(ret, g, k, a, b, A, F, min_over, Cb); }
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

// range = [l, r), return first value causing "t" in evalfunc that returns l->[f,...,f,t,...,t)->r
// NOTE: if [f,...,f) then return r, if [l, r) = empty set then invalid use
template <class val_t, class bsargv_t, class evalfunc_t>
val_t upper_bsearch(val_t l, val_t r, const bsargv_t& v, evalfunc_t evalfunc) {
    if (r - l == 1) {
        if (evalfunc(l, v))
            return l;
        else
            return r;
    }
    val_t m = (l + r) / 2;
    if (evalfunc(m, v))
        return upper_bsearch<val_t, bsargv_t>(l, m, v, evalfunc);
    else
        return upper_bsearch<val_t, bsargv_t>(m, r, v, evalfunc);
}

// list all minimal separators of size at most k
vector<BITSET> list_exact(const Graph& g, int k) {
    vector<BITSET> ret;
    hash_t h = get_hash(g);
    ADJSPRS g_adjsprs = encodeADJ(g.adj);
    size_t memo_offset = sep_dictionary.access(h);
    if (memo_offset != SIZE_MAX) {
        if (sep_dictionary.is_equal(memo_offset, g.nodes, g_adjsprs)) {
            if (size_t(k) <= sep_dictionary.get_k(memo_offset)) {
                auto [l, r] = sep_dictionary.get_sep_range(memo_offset);
                size_t m = upper_bsearch(l, r, static_memory, [&](auto i, const auto& a) { return a[i].count() > k; });
                ret.resize(m - l);
                std::copy(static_memory.begin() + l, static_memory.begin() + m, ret.begin());
                return ret;
            }
        }
    }
    int v = min_fill_vertex(g);
    BITSET X = open_neighbors(g, v);
    FOR_EACH(a, X) {
        FOR_EACH(b, X) {
            if (a == b) continue;
            if (!is_adjacent(g, a, b)) {
                BITSET A, F = intersection(open_neighbors(g, a), open_neighbors(g, b));
                auto Na = open_neighbors(g, a);
                auto Cb = components_contain(g, Na, b);
                A = components_contain(g, open_neighbors(g, Cb), a);
                if (get_min(A, &X) != a) continue;
                {
                    std::vector<BITSET> enum_buf;
                    enum_rec(enum_buf, g, k, a, b, A, F, &X, Cb);
                    vector_concat(ret, enum_buf);
                }
            }
        }
    }
    for (auto& conn : components(remove(g, X))) {
        if (conn.test(v)) continue;
        auto N = open_neighbors(g, conn);
        if (N.count() <= k) ret.push_back(N);
        {
            auto enum_buf = list_exact(local(g, join(conn, X)), k);
            vector_concat(ret, enum_buf);
        }
    }
    ret = unique(ret);
    if (ret.size() > 64) {
        sep_dictionary.insert(h, k, g.nodes, g_adjsprs, ret);
    }
    return ret;
}