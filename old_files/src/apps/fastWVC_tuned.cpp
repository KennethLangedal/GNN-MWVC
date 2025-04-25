#include "local_search.hpp"
#include <chrono>
#include <iostream>

using namespace std;

int main(int narg, char **arg) {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    if (narg != 2) {
        cout << "Usage: ./fastWVC_tuned [time]" << endl;
        return 0;
    }

    double t_max = stod(arg[1]);

    size_t E, N;

    cin >> E >> N;

    vector<uint32_t> weights(N), degree(N, 0);
    vector<pair<uint32_t, uint32_t>> edges(E);

    for (auto &&w : weights) {
        cin >> w;
    }

    for (size_t i = 0; i < E; ++i) {
        auto &&[u, v] = edges[i];
        cin >> u >> v;
        if (--u > --v)
            swap(u, v);
        if (u >= N || v >= N) {
            cout << "Invalid edge" << endl;
            return 0;
        }
        degree[u]++;
        degree[v]++;
    }
    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));
    E = edges.size();

    vector<optional<bool>> S(N, false);
    uint32_t cost = 0;

    for (auto &&[u, v] : edges) {
        if (!(*S[u]) && !(*S[v])) {
            if (((double)degree[u] / (double)weights[u]) > ((double)degree[v] / (double)weights[v])) {
                S[u] = true;
                cost += weights[u];
            } else {
                S[v] = true;
                cost += weights[v];
            }
        }
    }

    local_search ls(N, E, weights, edges, S);

    size_t i = 0;
    // vector<uint32_t> quality_at_time(100, 0);

    auto t0 = chrono::high_resolution_clock::now(), t1 = chrono::high_resolution_clock::now();

    // cout << "0.0," << ls.get_best_cost() << endl;

    size_t step_size = 1 << 16;
    while (chrono::duration<double>(chrono::high_resolution_clock::now() - t0).count() < t_max) {
        // step_size = ((t_max - chrono::duration<double>(chrono::high_resolution_clock::now() - t0).count()) / t_max) * 16.0;
        // step_size = 1ul << step_size;
        if (ls.search(step_size)) {
            t1 = chrono::high_resolution_clock::now();
            // cout << chrono::duration<double>(t1 - t0).count() << "," << ls.get_best_cost() << endl;
            step_size = min(step_size * 2, 1ul << 16);
            // cout << ls.get_best_cost() << " at step size " << step_size << "     " << flush << '\r';
        } else {
          step_size = max(step_size / 2, 1ul << 10);
          // cout << ls.get_best_cost() << " at step size " << step_size << "     " << flush << '\r';
        }

        // if ((i + 1) * 10 < chrono::duration<double>(chrono::high_resolution_clock::now() - t0).count()) {
        //     quality_at_time[i++] = ls.get_best_cost();
        // }
    }

    cout << ls.get_best_cost() << "," << chrono::duration<double>(t1 - t0).count() << endl;
    // for (auto &&c : quality_at_time) {
    //     cout << c << ",";
    // }
    // cout << endl;

    return 0;
}