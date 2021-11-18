#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

using namespace std;

optional<uint32_t> validate(const vector<uint32_t> &weights, const vector<pair<uint32_t, uint32_t>> &edges, const vector<bool> &vc) {
    for (auto &&[u, v] : edges) {
        if (!vc[u] && !vc[v]) {
            return nullopt;
        }
    }
    uint32_t cost = 0;
    for (size_t i = 0; i < weights.size(); ++i) {
        if (vc[i])
            cost += weights[i];
    }
    return cost;
}

int main(int narg, char **arg) {
    if (narg != 2) {
        cout << "Usage: ./approximation_solver [graph]" << endl;
        return 0;
    }

    ifstream is(arg[1]);
    if (!is.is_open()) {
        cout << "Error opening file" << endl;
        return 0;
    }

    size_t N, E;
    is >> E >> N;

    vector<uint32_t> weights(N);
    for (auto &&w : weights)
        is >> w;
    vector<uint32_t> SW = weights;

    vector<pair<uint32_t, uint32_t>> edges(E);
    for (auto &&[u, v] : edges) {
        is >> u >> v;
        if (--u > --v)
            swap(u, v);
    }
    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));

    vector<vector<uint32_t>> graph(N), g(N);

    vector<uint32_t> deactive_weights(N, 0);

    for (size_t i = 0; i < edges.size(); ++i) {
        auto &&[u, v] = edges[i];
        graph[u].push_back(i);
        graph[v].push_back(i);

        g[u].push_back(v);
        g[v].push_back(u);

        deactive_weights[u] += weights[v];
        deactive_weights[v] += weights[u];
    }

    vector<bool> active(edges.size(), true), vc(N, false), tmp_active(N, false);

    for (size_t i = 0; i < edges.size(); ++i) {
        if (!active[i])
            continue;

        auto &&[u, v] = edges[i];
        uint32_t k = SW[u] < SW[v] ? u : v;
        SW[u] -= SW[k];
        SW[v] -= SW[k];
        vc[k] = true;
        for (auto j : g[k])
            deactive_weights[j] -= weights[k];
        for (auto j : graph[k])
            active[j] = false;
    }

    auto remove_node_from_vc = [&](uint32_t u) {
        vc[u] = false;
        for (auto &&v : g[u]) {
            deactive_weights[v] += weights[u];
            if (!vc[v]) {
                vc[v] = true;
                for (auto &&w : g[v]) {
                    deactive_weights[w] -= weights[v];
                }
            }
        }
    };

    bool improvement = true;
    while (improvement) {
        improvement = false;
        for (size_t i = 0; i < N; ++i) {
            if (!vc[i]) {
                uint32_t cost_improvement = 0;
                for (auto &&v : g[i])
                    tmp_active[v] = true;
                for (auto &&v : g[i]) {
                    if (!tmp_active[v])
                        continue;
                    if (deactive_weights[v] - weights[i] < weights[v]) {
                        cost_improvement += weights[v] - (deactive_weights[v] - weights[i]);
                        for (auto &&w : g[v])
                            tmp_active[w] = false;
                    } else {
                        tmp_active[v] = false;
                    }
                }
                if (cost_improvement > weights[i]) {
                    improvement = true;
                    for (auto &&v : g[i]) {
                        if (tmp_active[v]) {
                            remove_node_from_vc(v);
                        }
                    }
                }
                for (auto &&v : g[i])
                    tmp_active[v] = false;
            } else {
                if (deactive_weights[i] < weights[i]) {
                    improvement = true;
                    remove_node_from_vc(i);
                }
            }
        }
    }

    auto cost = validate(weights, edges, vc);
    if (cost) {
        cout << "Valid cover, cost: " << *cost << endl;
    } else {
        cout << "Not a valid vertex cover" << endl;
    }

    return 0;
}