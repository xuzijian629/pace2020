#pragma once
#include "graph.cpp"

unordered_set<int> open_neighbors(const Graph& g, int v) {
    unordered_set<int> ret;
    for (int a : g.adj.at(v)) ret.insert(a);
    return ret;
}

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
    unordered_set<int> ret;
    if (A.size() > B.size()) return intersection(B, A);
    for (int a : A) {
        if (B.count(a)) ret.insert(a);
    }
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
        auto X = close_neighbors(g, v);
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

int get_min(const unordered_set<int>& C) {
    assert(!C.empty());
    int ret = 1e9;
    for (int a : C) ret = min(ret, a);
    return ret;
}

// a \subset b?
bool is_subset(const unordered_set<int>& a, const unordered_set<int>& b) {
    for (int c : a) {
        if (!b.count(c)) return false;
    }
    return true;
}

unordered_set<int> get_close_sep(const Graph& g, const unordered_set<int>& A, int b, int v) {
    unordered_set<int> C(A);
    C.insert(v);
    for (auto& conn : components(remove(g, close_neighbors(g, C)))) {
        if (conn.nodes.count(b)) {
            auto N = open_neighbors(g, conn.nodes);
            for (auto& A_ : components(remove(g, N))) {
                if (A_.nodes.count(v)) return A_.nodes;
            }
        }
    }
    // not-found (for example, when b and v are connected)
    return unordered_set<int>();
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

vector<unordered_set<int>> enum_rec(const Graph& g, int k, int a, int b, const unordered_set<int>& A,
                                    const unordered_set<int>& F) {
    vector<unordered_set<int>> ret;
    if (F.size() > k) return ret;
    if (a != get_min(A)) return ret;
    unordered_set<int> Cb;
    for (auto& conn : components(remove(g, close_neighbors(g, A)))) {
        if (conn.nodes.count(b)) {
            Cb = conn.nodes;
            break;
        }
    }
    assert(!Cb.empty());
    if (b != get_min(Cb)) return ret;
    if (!is_subset(F, open_neighbors(g, Cb))) return ret;
    auto Na = open_neighbors(g, A);
    if (F.size() == k && Na != F) return ret;
    if (A.size() > Cb.size()) return ret;
    if (Na.size() > k && A.size() + (Na.size() - k) > min((int)Cb.size(), (g.n - k) / 2)) return ret;
    // Lemma 4 による枝刈りは未追加

    // F.size() == k は論文の誤植？スライドは F.size() <= k になっている
    if (F.size() <= k && Na == F && open_neighbors(g, Cb) == F && A.size() <= Cb.size()) {
        ret.push_back(Na);
        return ret;
    }

    auto dif = difference(Na, F);
    if (dif.empty()) return ret;
    for (int v : dif) {
        auto F_(F);
        F_.insert(v);
        for (auto& s : enum_rec(g, k, a, b, A, F_)) {
            ret.push_back(s);
        }
        auto A_ = get_close_sep(g, A, b, v);
        if (A_.empty()) continue;
        for (auto& s : enum_rec(g, k, a, b, A_, F)) {
            ret.push_back(s);
        }
    }
    return unique(ret);
}

Graph local(const Graph& g, Graph C) {
    for (auto& D : components(remove(g, C.nodes))) {
        auto N = open_neighbors(g, D.nodes);
        for (int a : N) {
            for (int b : N) {
                if (a == b) continue;
                C.add_edge(a, b);
            }
        }
    }
    return C;
}

vector<unordered_set<int>> list_exact_slow(const Graph& g, int k) {
    vector<unordered_set<int>> ret;
    for (int a : g.nodes) {
        for (int b : g.nodes) {
            if (a == b) continue;
            // a と b の順序は重要
            if (!is_adjacent(g, a, b)) {
                unordered_set<int> A, F;
                A.insert(a);
                auto Na = open_neighbors(g, a);
                auto Nb = open_neighbors(g, b);
                F = intersection(Na, Nb);
                for (auto& s : enum_rec(g, k, a, b, A, F)) {
                    ret.push_back(s);
                }
            }
        }
    }
    return unique(ret);
}

// list all minimal separators of size at most k
vector<unordered_set<int>> list_exact(const Graph& g, int k) {
    vector<unordered_set<int>> ret;
    int v = min_fill_vertex(g);
    unordered_set<int> X = close_neighbors(g, v);
    // X は separator でなくてもよい？
    for (int a : X) {
        for (int b : X) {
            if (a == b) continue;
            // a と b の順序は重要
            if (!is_adjacent(g, a, b)) {
                unordered_set<int> A, F;
                A.insert(a);
                auto Na = open_neighbors(g, a);
                auto Nb = open_neighbors(g, b);
                F = intersection(Na, Nb);
                for (auto& s : enum_rec(g, k, a, b, A, F)) {
                    ret.push_back(s);
                }
            }
        }
    }

    for (auto& conn : components(remove(g, X))) {
        for (auto& s : list_exact(local(g, conn), k)) {
            ret.push_back(s);
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
