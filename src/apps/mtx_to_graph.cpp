#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

using namespace std;

int main(int narg, char **arg) {
    if (narg != 3) {
        cout << "Usage: ./mtx_to_graph [graph] [output graph]" << endl;
        return 0;
    }

    string in_path(arg[1]), out_path(arg[2]);

    ifstream is(in_path);
    ofstream os(out_path);

    if (!is.is_open() || !os.is_open()) {
        cout << "Error opening files" << endl;
        return 0;
    }

    size_t N, E, w, u, v;

    is >> E >> N;

    vector<uint32_t> weights(N);
    vector<vector<uint32_t>> edges(N);

    for (size_t i = 0; i < N; ++i) {
        is >> weights[i];
    }
    for (size_t i = 0; i < E; ++i) {
        is >> u >> v;
        edges[u - 1].push_back(v);
        edges[v - 1].push_back(u);
    }
    for (size_t i = 0; i < N; ++i) {
        sort(begin(edges[i]), end(edges[i]));
    }
    os << N << " " << E << " 10" << endl;
    for (size_t i = 0; i < N; ++i) {
        os << weights[i] << " ";
        for (size_t j = 0; j < edges[i].size(); ++j) {
            os << edges[i][j] << " ";
        }
        os << endl;
    }
    return 0;
}