#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stack>
#include <vector>

struct Tree {
    std::vector<int> parent;
    int depth;
};

using Edge = std::pair<int, int>;

struct Graph {
    std::vector<Edge> edges;
    std::size_t n;
};

/////////////////// CHECK IF TREE IS INDEED TREE /////

// Returns false if the graph contains a cycle, else true.
bool is_tree(Tree const &t) {
    auto n = t.parent.size();
    std::vector<int> visited(n, 0);

    for (int u = 1; u < n; u++) {
        if (visited[u] == 0) {
            visited[u] = u;
            auto v = t.parent[u];
            while (v != 0) {
                if (visited[v] == u) {
                    return false;
                }
                if (visited[v] > 0) {
                    break;
                }
                visited[v] = u;
                v = t.parent[v];
            }
        }
    }

    return true;
}

// helper function for treedepth
int _tree_depth(Tree const &t, std::vector<int> &depth, int vertex) {
    std::stack<int> d({vertex});
    while (depth[d.top()] == -1 && d.top() != 0) {
        d.push(t.parent[d.top()]);
    }

    auto curr = depth[d.top()];
    d.pop();
    while (!d.empty()) {
        depth[d.top()] = ++curr;
        d.pop();
    }
    return curr;
}

int tree_depth(Tree const &t) {
    std::vector<int> depth(t.parent.size(), -1);

    int max = 0;
    for (int u = 1; u < t.parent.size(); u++) {
        max = std::max(max, _tree_depth(t, depth, u));
    }

    return max + 1;
}

///////////////////// VERIFY //////////////////////

bool verify_edge(Edge const &e, Tree const &tree) {
    auto start = e.second;
    while (start != 0) {
        if (start == e.first) {
            return true;
        }
        start = tree.parent[start];
    }
    return false;
}

bool verify(Graph const &g, Tree const &t) {
    if (t.parent.size() - 1 != g.n) {
        std::cerr << "Tree has incorrect nr of nodes! tree size= " << t.parent.size() - 1 << " vs. graph size = " << g.n
                  << "\n";
        return false;
    }

    if (!is_tree(t)) {
        std::cerr << "Tree has cycles!\n";
        return false;
    }
    for (auto const &e : g.edges) {
        if (!(verify_edge(e, t) || verify_edge(Edge(e.second, e.first), t))) {
            std::cerr << "Error, edge " << e.first << " " << e.second << "\n";
            return false;
        }
    }

    if (tree_depth(t) != t.depth) {
        std::cerr << "Tree has incorrect tree depth! Real " << tree_depth(t) << " vs. declared" << t.depth << "\n";
        return false;
    }

    return true;
}

//////////////////// TESTS ///////////////////////////////

bool test_accept() {
    //-1 in the beginning since vertices starts with 1
    Tree tree{{-1, 0, 1, 1, 2, 3}, 3};
    Graph g{{{1, 2}, {1, 3}, {2, 4}, {3, 5}, {1, 4}, {1, 5}}, 5};

    return verify(g, tree);
}

bool test_incorrect() {
    //-1 in the beginning since vertices starts with 1
    Tree tree{{-1, 0, 1, 1, 1, 1}, -1};
    Graph g{{{1, 2}, {1, 3}, {2, 4}, {3, 5}, {1, 4}, {1, 5}}, 5};

    return !verify(g, tree);
}

bool test_cycle() {
    //-1 in the beginning since vertices starts with 1
    Tree tree{{-1, 2, 1, 1, 2, 1}, -1};
    Graph g{{{1, 2}, {1, 3}, {2, 4}, {3, 5}, {1, 4}, {1, 5}}, 5};

    return !verify(g, tree);
}

bool test_cycle_big() {
    //-1 in the beginning since vertices starts with 1
    Tree tree{{-1, 2, 3, 4, 5, 1}, -2};
    Graph g{{{1, 2}, {1, 3}, {2, 4}, {3, 5}, {1, 4}, {1, 5}}, 5};

    return !verify(g, tree);
}

void run_tests() {
    std::cout << "Testing (2 cycle errors expected)" << std::endl;
    assert(test_accept());
    assert(test_incorrect());
    assert(test_cycle());
    assert(test_cycle_big());
}

////////////////////////////////// READ DATA /////////////////////////

Graph read_graph(std::istream &stream) {
    std::size_t vertex_count = 0;
    std::size_t edge_count = 0;

    bool firstLine = true;
    bool error = false;

    Graph g;

    if (stream.good()) {
        std::string line;
        std::size_t pos = 0;

        while (!error && std::getline(stream, line)) {
            if (line.empty()) {
                error = true;
            } else {
                if (line.back() == '\r') {
                    line.pop_back();
                }

                if (line[0] != 'c') {
                    if (firstLine) {
                        auto head = std::string("p tdp ");
                        if (line.compare(0, head.size(), head) != 0) {
                            error = true;
                        }

                        line = line.substr(head.size());

                        g.n = std::stol(line, &pos);

                        if (line[pos] != ' ') {
                            error = true;
                        }

                        line = line.substr(pos + 1);

                        edge_count = std::stol(line, &pos);

                        if (pos != line.length()) {
                            error = true;
                        }

                        firstLine = false;
                    } else {
                        auto vertex1 = std::stoul(line, &pos);

                        if (line[pos] != ' ') {
                            error = true;
                        } else {
                            line = line.substr(pos + 1);

                            auto vertex2 = std::stoul(line, &pos);

                            if (pos != line.length()) {
                                error = true;
                            }

                            g.edges.push_back(Edge(vertex1, vertex2));

                            edge_count--;
                        }
                    }
                }
            }
        }

        if (edge_count != 0) {
            error = true;
        }
    } else {
        error = true;
    }

    if (firstLine || error) {
        std::cerr << "Error while parsing graph instance\n";
        return {};
    }

    return g;
}

Tree read_tree(std::istream &stream) {
    bool error = false;
    //-1 in the beginning since vertices starts with 1
    Tree t = {{-1}, -1};

    if (stream.good()) {
        std::string line;
        std::size_t pos = 0;
        if (!std::getline(stream, line)) {
            error = true;
        }
        t.depth = std::stol(line);

        while (!error && std::getline(stream, line)) {
            if (line.empty()) {
                error = true;
            }
            t.parent.push_back(std::stol(line));
        }
    }

    if (error) {
        std::cerr << "Error while parsing tree instance\n";
        return {};
    }
    return t;
}

int main(int argc, char *argv[]) {
    // run_tests();

    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " graph_file.gr tree_depth_decomposition_file.tree\n";
        return -1;
    }

    std::ifstream if_graph(argv[1], std::ifstream::in);
    std::ifstream if_tree(argv[2], std::ifstream::in);

    auto g = read_graph(if_graph);
    auto t = read_tree(if_tree);
    if (verify(g, t)) {
        std::cout << t.depth << "|SUCCEED" << std::endl;
        return 0;
    } else {
        std::cout << "0|FAILED" << std::endl;
        return -2;
    }
}
