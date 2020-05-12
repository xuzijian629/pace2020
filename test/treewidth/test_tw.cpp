#include "../../src/lib/lower_bound.cpp"
#include "../../src/lib/tw.cpp"

int main() {
    Graph g = read_input();
    int tw = treewidth_exact(g);
    cout << tw << endl;
}
