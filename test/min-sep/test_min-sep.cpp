#include "../../src/lib/minimal_separator.cpp"

int main() {
    Graph g = read_input();
    auto minseps = list_exact(g, 10);
    assert(is_separators(g, minseps));
    vector<vector<int>> sol;
    for (auto& s : minseps) {
        vector<int> sep(s.begin(), s.end());
        sort(sep.begin(), sep.end());
        sol.push_back(sep);
    }
    sort(sol.begin(), sol.end(), [](auto& a, auto& b) { return (a.size() == b.size() ? a < b : a.size() < b.size()); });
    cout << sol.size() << endl;
    for (auto& s : sol) {
        cout << s.size();
        for (int a : s) {
            cout << ' ' << a + 1;
        }
        cout << endl;
    }
}
