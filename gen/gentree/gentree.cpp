#include <bits/stdc++.h>

using namespace std;

using vi = vector<int>;
using vvi = vector<vi>;
using vvvi = vector<vvi>;
using ll = long long int;
using vll = vector<ll>;
using vvll = vector<vll>;
using vvvll = vector<vvll>;
using vd = vector<double>;
using vvd = vector<vd>;
using vvvd = vector<vvd>;
using P = pair<int, int>;
using Pll = pair<ll, ll>;
using cdouble = complex<double>;

const double eps = 1e-7;
#define Loop(i, n) for (int i = 0; i < int(n); i++)
#define Loopll(i, n) for (ll i = 0; i < ll(n); i++)
#define Loop1(i, n) for (int i = 1; i <= int(n); i++)
#define Loopll1(i, n) for (ll i = 1; i <= ll(n); i++)
#define Loopr(i, n) for (int i = int(n) - 1; i >= 0; i--)
#define Looprll(i, n) for (ll i = ll(n) - 1; i >= 0; i--)
#define Loopr1(i, n) for (int i = int(n); i >= 1; i--)
#define Looprll1(i, n) for (ll i = ll(n); i >= 1; i--)
#define Foreach(buf, container) for (const auto &buf : container)
#define Foreachr(buf, container) for (const auto &buf : reversed(container))
#define Loopdiag(i, j, h, w, sum) for (int i = ((sum) >= (h) ? (h)-1 : (sum)), j = (sum)-i; i >= 0 && j < (w); i--, j++)
#define Loopdiagr(i, j, h, w, sum) \
    for (int j = ((sum) >= (w) ? (w)-1 : (sum)), i = (sum)-j; j >= 0 && i < (h); j--, i++)
#define Loopdiagsym(i, j, h, w, gap) for (int i = ((gap) >= 0 ? (gap) : 0), j = i - (gap); i < (h) && j < (w); i++, j++)
#define Loopdiagsymr(i, j, h, w, gap) \
    for (int i = ((gap) > (h) - (w)-1 ? (h)-1 : (w)-1 + (gap)), j = i - (gap); i >= 0 && j >= 0; i--, j--)
#define Loopitr(itr, container) for (auto itr = container.begin(); itr != container.end(); itr++)
#define quickio()                     \
    std::ios::sync_with_stdio(false); \
    std::cin.tie(0);
#define bitmanip(m, val) static_cast<bitset<(int)m>>(val)
#define Comp(type_t) bool operator<(const type_t &another) const
#define fst first
#define snd second
#define INF INFINITY
bool feq(double x, double y) { return abs(x - y) <= eps; }
bool inrange(ll x, ll t) { return x >= 0 && x < t; }
bool inrange(vll xs, ll t) {
    Foreach(x, xs) if (!(x >= 0 && x < t)) return false;
    return true;
}
int ceillog2(ll x) {
    int m = int(log2(x));
    return m + ((1LL << m) < x ? 1 : 0);
}
int floorlog2(ll x) {
    int m = int(log2(x));
    return m - ((1LL << m) > x ? 1 : 0);
}
template <class T>
T reversed(T container) {
    reverse(container.begin(), container.end());
    return container;
}
template <class T>
void printv(const vector<T> &v) {
    for (const T &x : v) cout << x << " ";
    cout << endl;
}
template <class T>
void printmx(const vector<vector<T>> &mx) {
    for (const vector<T> &v : mx) printv(v);
}
ll rndf(double x) { return (ll)(x + (x >= 0 ? 0.5 : -0.5)); }
ll floorsqrt(ll x) {
    ll m = (ll)sqrt((double)x);
    return m + (m * m <= x ? 0 : -1);
}
ll ceilsqrt(ll x) {
    ll m = (ll)sqrt((double)x);
    return m + (x <= m * m ? 0 : 1);
}
ll rnddiv(ll a, ll b) { return (a / b + (a % b * 2 >= b ? 1 : 0)); }
ll ceildiv(ll a, ll b) { return (a / b + (a % b == 0 ? 0 : 1)); }
ll gcd(ll m, ll n) {
    if (n == 0)
        return m;
    else
        return gcd(n, m % n);
}
ll lcm(ll m, ll n) { return ll(m) * ll(n) / gcd(m, n); }

//========================================================================//

class Union_Find {
private:
    vi p, r;  // parent, rank
    vll w;    // weight
    ll (*unite_rule)(ll, ll);
    // w is the size of connected component in default
    static ll default_unite_rule(ll u, ll v) { return u + v; }

public:
    Union_Find(int N, const vll w = {}, ll (*unite_rule)(ll, ll) = nullptr) {
        this->p.resize(N);
        this->r.resize(N);
        this->w.resize(N);
        Loop(i, N) {
            this->p[i] = i;
            this->r[i] = 0;
            this->w[i] = w.size() ? w[i] : 1;
        }
        this->unite_rule = (unite_rule != nullptr ? unite_rule : default_unite_rule);
    }
    int find(int x) {
        if (p[x] == x)
            return x;
        else
            return p[x] = find(p[x]);
    }
    void unite(int x, int y) {
        x = find(x);
        y = find(y);
        if (x == y) return;
        if (r[x] == r[y]) r[x]++;
        if (r[x] < r[y]) {
            p[x] = y;
            w[y] = unite_rule(w[x], w[y]);
        } else {
            p[y] = x;
            w[x] = unite_rule(w[x], w[y]);
        }
    }
    bool is_same(int x, int y) { return find(x) == find(y); }
    int get_w(int x) { return w[find(x)]; }
    void change_w(int x, ll v) { w[find(x)] = v; }
};

class Random_Int {
private:
    std::mt19937 *mt;
    std::uniform_int_distribution<> *distr_int;

public:
    // uniform int distribution of [0, m)
    Random_Int(int m) {
        mt = new std::mt19937((uint64_t)(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                             std::chrono::high_resolution_clock::now().time_since_epoch())
                                             .count()));
        distr_int = new std::uniform_int_distribution<>(0, m - 1);
    }
    int get() { return (*distr_int)(*mt); }
};

int main() {
    for (int N = 50; N < 200; ++N) {
        if (N < 1) return 0;
        std::ofstream ofs;
        ofs.open("tree" + std::string(N < 100 ? "0" : "") + std::to_string(N) + ".gr");
        if (!ofs.is_open()) continue;
        vector<Random_Int *> ri(N + 1);
        for (int i = 1; i < N + 1; ++i) {
            ri[i] = new Random_Int(i);
        }
        vector<int> ids(N);
        vector<vector<int>> ccs(N);
        for (int i = 0; i < N; ++i) {
            ids[i] = i;
            ccs[i] = {i};
        }
        vector<pair<int, int>> edges(N - 1);
        for (int _ = 0; _ < N - 1; ++_) {
            int posx = ri[ids.size()]->get();
            int posy = ri[ids.size()]->get();
            if (posx == posy) posy = (posx + 1) % ids.size();
            int px = ids[posx];
            int py = ids[posy];
            int x = ccs[px][ri[ccs[px].size()]->get()];
            int y = ccs[py][ri[ccs[py].size()]->get()];
            edges[_] = {x, y};
            for (int z : ccs[py]) {
                ccs[px].push_back(z);
            }
            ccs[py].clear();
            ids.erase(ids.begin() + posy);
        }
        Union_Find *uf = new Union_Find(N);
        ofs << "p tdp " << N << " " << N - 1 << endl;
        for (auto edge : edges) {
            uf->unite(edge.first, edge.second);
            ofs << edge.first + 1 << " " << edge.second + 1 << endl;
        }
    }
}