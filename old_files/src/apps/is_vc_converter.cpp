#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <optional>

using namespace std;

using Tn = uint32_t;
using Tw = uint32_t;

optional<Tw> validate(const vector<Tw> &weights, const vector<pair<Tn, Tn>> &edges, const vector<Tn> &s) {
    for (auto [u, v] : edges) {
        if (s[u] == 0 && s[v] == 0)
            return nullopt;
    }
    Tw cost = 0;
    for (Tn u = 0; u < weights.size(); u++) {
        if (s[u])
            cost += weights[u];
    }
    return cost;
}

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    if (argc != 4) {
        cout << "Validates independent set and converts it to a vertex cover" << endl;
        cout << "Usage: is_vc_converter [graph] [solution] [output]" << endl;
        return 0;
    }

    string graph_path = argv[1], solution_path = argv[2], out_path = argv[3];
    
    ifstream gs(graph_path, ios::in);

    uint64_t E, N;
    gs >> E >> N;
    vector<Tw> weights(N);
    for (auto &&w : weights)
        gs >> w;
    vector<pair<Tn, Tn>> edges(E);
    for (auto &&[u, v] : edges) {
        gs >> u >> v;
        if (--u > --v)
            swap(u, v);
    }

    vector<Tn> s(N);
    ifstream ss(solution_path, ios::in);
    for (auto &&v : s) {
        ss >> v;
        v = v > 0 ? 0 : 1;
    }

    auto cost = validate(weights, edges, s);
    if (!cost.has_value()) {
        cout << "Not a vertex cover!" << endl;
    } else {
        cout << "Valid vertex cover, cost: " << *cost << endl;
        ofstream os(out_path, ios::out);
        for (auto &&v : s)
            os << v << endl;
    }

    return 0;
}