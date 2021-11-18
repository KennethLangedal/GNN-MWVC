#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main(int narg, char **arg) {
    if (narg != 6) {
        cout << "Usage: ./gen_weights [graph] [output graph] [min] [max] [seed]" << endl;
        return 0;
    }

    string in_path(arg[1]), out_path(arg[2]), line;
    size_t min = stoi(arg[3]), max = stoi(arg[4]);
    int seed = stoi(arg[5]);

    ifstream is(in_path);
    ofstream os(out_path);

    if (!is.is_open() || !os.is_open()) {
        cout << "Error opening files" << endl;
        cout << in_path << endl
             << out_path << endl;
        return 0;
    }

    getline(is, line);
    while (line.front() == '%')
        getline(is, line);
    size_t N, E, w;
    stringstream ss(line);
    ss >> N >> N >> E;

    vector<pair<uint32_t, uint32_t>> edges;
    vector<uint32_t> weights(N);
    mt19937 reng(seed == -1 ? N : seed);
    uniform_int_distribution<uint32_t> dist(min, max);

    uint32_t u, v;
    for (size_t i = 0; i < E; ++i) {
        getline(is, line);
        ss.str(line);
        ss >> u >> v;
        if (u != v)
            edges.push_back({std::min(u, v), std::max(u, v)});
        ss.clear();
    }

    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));

    os << edges.size() << " " << N << endl;
    for (size_t i = 0; i < N; ++i) {
        // is >> w;
        os << dist(reng) << " ";
    }
    os << endl;

    for (auto &&[u, v] : edges)
        os << u << " " << v << endl;

    return 0;
}