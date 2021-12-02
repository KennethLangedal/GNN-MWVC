#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

vector<vector<pair<size_t, size_t>>> g;
vector<pair<size_t, size_t>> edges;
vector<size_t> heap, heap_index, dscore, w, edge_w, age;
vector<bool> S, best_S, conf, tabu;

size_t N, E, cost, best_cost, step, previous_top;

constexpr double eps = 0.000001;

bool compare_nodes(size_t u, size_t v) {
    if (!conf[u] || tabu[u] || !S[u])
        return false;
    if (!conf[v] || tabu[v] || !S[v])
        return true;
    double us = (double)w[u] / ((double)dscore[u] + eps), vs = (double)w[v] / ((double)dscore[v] + eps);
    return us > vs || (!(vs > us) && age[u] < age[v]);
}

size_t heap_top() {
    return heap[0];
}

void move_up(size_t u) {
    size_t i = heap_index[u];
    size_t p = (i - 1) / 2;
    while (i != 0 && compare_nodes(heap[i], heap[p])) {
        swap(heap_index[heap[i]], heap_index[heap[p]]);
        swap(heap[i], heap[p]);
        i = p;
        p = (i - 1) / 2;
    }
}

void move_down(size_t u) {
    size_t i = heap_index[u];
    size_t l = (i * 2) + 1, r = (i * 2) + 2;
    size_t min_child = (r >= N || compare_nodes(heap[l], heap[r])) ? l : r;
    while (min_child < N && compare_nodes(heap[min_child], heap[i])) {
        swap(heap_index[heap[i]], heap_index[heap[min_child]]);
        swap(heap[i], heap[min_child]);
        i = min_child;
        l = (i * 2) + 1, r = (i * 2) + 2;
        min_child = (r >= N || compare_nodes(heap[l], heap[r])) ? l : r;
    }
}

void make_heap() {
    for (size_t i = N; i > 0; --i) {
        move_down(heap[i - 1]);
    }
}

void update_node_in_heap(size_t u) {
    move_up(u);
    move_down(u);
}

void init_solutions() {
    for (auto &&[u, v] : edges) {
        if (!S[u] && !S[v]) {
            if (((double)g[u].size() / (double)w[u]) > ((double)g[v].size() / (double)w[v])) {
                S[u] = true;
                cost += w[u];
            } else {
                S[v] = true;
                cost += w[v];
            }
        }
    }
    for (auto &&[u, v] : edges) {
        if (S[u] && !S[v]) {
            dscore[u]++;
        } else if (!S[u] && S[v]) {
            dscore[v]++;
        }
    }
    for (size_t u = 0; u < N; ++u) {
        if (S[u] && dscore[u] == 0) {
            S[u] = false;
            cost -= w[u];
            for (auto &&[v, id] : g[u]) {
                dscore[v]++;
            }
        }
    }
    copy(begin(S), end(S), begin(best_S));
    best_cost = cost;
}

bool local_search(size_t steps = 10000) {
    for (size_t i = 0; i < steps; ++i) {
        size_t u = heap_top();
        if (previous_top != N) {
            for (auto &&[v, id] : g[previous_top]) {
                if (tabu[v]) {
                    tabu[v] = false;
                    update_node_in_heap(v);
                }
            }
        }
        previous_top = u;
        S[u] = false;
        cost -= w[u];
        dscore[u] = 0;
        step++;
        age[u] = step;
        update_node_in_heap(u);
        sort(begin(g[u]), end(g[u]), [&](auto &&a, auto &&b) {
            return ((double)edge_w[a.second] / (double)w[a.first]) > ((double)edge_w[b.second] / (double)w[b.first]);
        });

        size_t count = 1;
        for (auto &&[v, id] : g[u]) {
            if (!S[v]) {
                conf[v] = true;
                tabu[v] = true;
                age[v] = step;
                S[v] = true;
                cost += w[v];
                edge_w[id] += count;
                dscore[v] = edge_w[id];
                update_node_in_heap(v);
                for (auto &&[w, _id] : g[v]) {
                    if (w == u)
                        continue;
                    dscore[w] -= edge_w[_id];
                    conf[w] = true;
                    update_node_in_heap(w);
                }
                count++;
            } else {
                dscore[v] += edge_w[id];
                update_node_in_heap(v);
            }
        }
    }

    if (cost < best_cost) {
        best_cost = cost;
        copy(begin(S), end(S), begin(best_S));
        return true;
    }
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    cin >> E >> N;
    g.resize(N);
    edges.resize(E);
    heap = vector<size_t>(N), heap_index = vector<size_t>(N), dscore = vector<size_t>(N, 0), w = vector<size_t>(N), edge_w = vector<size_t>(E, 1), age = vector<size_t>(N, 0);
    S = vector<bool>(N, false), best_S = vector<bool>(N, false), conf = vector<bool>(N, true), tabu = vector<bool>(N, false);
    cost = 0, best_cost = 0, step = 0;
    iota(begin(heap), end(heap), 0);
    iota(begin(heap_index), end(heap_index), 0);

    for (auto &&_w : w) {
        cin >> _w;
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
        // g[--u].push_back({--v, i});
        // g[v].push_back({u, i});
    }
    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));
    E = edges.size();
    for (size_t i = 0; i < E; ++i) {
        auto &&[u, v] = edges[i];
        g[u].push_back({v, i});
        g[v].push_back({u, i});
    }

    auto t0 = chrono::high_resolution_clock::now(), t1 = chrono::high_resolution_clock::now();

    init_solutions();

    make_heap();

    previous_top = N;

    size_t step_size = 1 << 16;
    while (chrono::duration<double>(chrono::high_resolution_clock::now() - t0).count() < 1000) {
        // local_search();
        if (local_search(step_size)) {
            t1 = chrono::high_resolution_clock::now();
            step_size = min(step_size * 2, 1ul << 16);
            cerr << best_cost << flush << '\r';
        } else {
            step_size = max(step_size / 2, 256ul);
        }
    }

    // cout << best_cost << " " << chrono::duration<double>(t1 - t0).count() << "s " << step << " steps" << endl;
    cout << best_cost << "," << chrono::duration<double>(t1 - t0).count() << endl;

    // cout << best_cost << endl;
    // for (size_t u = 0; u < N; ++u) {
    //     if (best_S[u])
    //         cout << u << " ";
    // }
    // cout << endl;

    return 0;
}
