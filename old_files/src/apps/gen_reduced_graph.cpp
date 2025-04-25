#include "mwvc_reductions.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int narg, char **arg) {
    if (narg != 3) {
        cout << "Usage: ./gen_reduced_graph [graph] [output graph]" << endl;
        return 0;
    }

    string in_path(arg[1]), out_path(arg[2]);

    ifstream is(in_path);

    if (!is.is_open()) {
        cout << "Error opening files" << endl;
        return 0;
    }

    size_t N, E;
    is >> E >> N;

    vector<uint32_t> weights(N);
    for (auto &&w : weights)
        is >> w;

    vector<pair<uint32_t, uint32_t>> edges(E);
    for (auto &&[u, v] : edges) {
        is >> u >> v;
        if (--u > --v)
            swap(u, v);
    }
    sort(begin(edges), end(edges));
    reduction_graph<uint32_t, uint32_t> g(weights, edges);
    graph_search<uint32_t> gs(N, 3);
    vertex_cover<uint32_t, uint32_t> vc(N);

    cout << in_path << ", before: " << g.size() << ", after: ";

    reduce_graph(g, vc, gs, false);
    g.relable_graph();

    cout << g.size() << endl;
    if (g.size() == 0) {
        cout << "Cost " << vc.cost << endl;
        return 0;
    }
    edges.clear();
    for (uint32_t u = 0; u < g.size(); ++u) {
        for (auto &&v : g[u]) {
            if (u < v)
                edges.push_back({u, v});
        }
    }
    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));

    ofstream os(out_path);
    os << edges.size() << " " << g.size() << endl;
    for (uint32_t u = 0; u < g.size(); ++u) {
        os << g.W(u) << " ";
    }
    os << endl;
    for (auto &&[u, v] : edges) {
        os << u + 1 << " " << v + 1 << endl;
    }

    return 0;
}