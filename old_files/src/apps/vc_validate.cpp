#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

using namespace std;

int main(int narg, char **arg) {
    if (narg != 3) {
        cout << "Uasge: ./vc_validate [graph] [solution]" << endl;
    }

    ifstream gs(arg[1]), ss(arg[2]);
    if (!gs.is_open() || !ss.is_open()) {
        cout << "Error opening files" << endl;
        return 0;
    }

    filesystem::path p(arg[1]);
    string name = p.filename().stem();

    size_t E, N;
    gs >> E >> N;

    vector<size_t> weights(N);
    for (auto &&w : weights)
        gs >> w;

    vector<pair<size_t, size_t>> edges(E);
    for (auto &&[u, v] : edges) {
        gs >> u >> v;
        if (u > N || v > N || v == u) {
            cout << "Invalid edge: " << u << "," << v << endl;
            return 0;
        }
        if (--u < --v)
            swap(u, v);
    }

    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));

    if (edges.size() != E) {
        cout << "Graph contained duplicate edges" << endl;
        return 0;
    }

    vector<bool> s(N);
    size_t b, cost = 0;
    for (size_t i = 0; i < N; ++i) {
        ss >> b;
        s[i] = b == 1;
        if (s[i])
            cost += weights[i];
    }

    for (auto &&[u, v] : edges) {
        if (!s[u] && !s[v]) {
            cout << "Edge " << u + 1 << "," << v + 1 << " is not covered" << endl;
            return 0;
        }
    }
    cout << "Valid vertex cover, graph " << name << ", cost: " << cost << endl;
    return 0;
}