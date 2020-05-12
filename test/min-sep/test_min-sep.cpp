#include "../../src/lib/lower_bound.cpp"
#include "../../src/lib/minimal_separator.cpp"

int main(int argc, char** argv) {
    int max_n = argc == 1 ? 10 : atoi(argv[1]);
    Graph g = read_input();
    auto minseps = list_exact(g, max_n);
    assert(is_separators(g, minseps));
    vector<vector<int>> sol;
    for (auto& s : minseps) {
        vector<int> sep;
        FOR_EACH(v, s) sep.push_back(v);
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
