#include "../../src/lib/minimal_separator.cpp"
#include "../../src/lib/precompute.cpp"

Graph treedepth_decomp(Graph g) {
    // dummy
    return Graph();
}

int main() {
    auto g = read_input();
    int k = 10;
    prepare_dist(g);
    vector<BITSET> clusters = kmeans(g, k);
    BITSET all;
    for (int i = 0; i < k; i++) {
        all |= clusters[i];
        for (int j = i + 1; j < k; j++) {
            assert(intersection(clusters[i], clusters[j]).none());
        }
    }
    FOR_EACH(v, g.nodes) assert(all.test(v));

    for (int i = 0; i < k; i++) {
        cout << "size of cluster " << i << ": " << clusters[i].count() << endl;
    }
}
