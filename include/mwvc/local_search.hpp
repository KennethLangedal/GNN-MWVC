#pragma once
#include <algorithm>
#include <numeric>
#include <vector>

constexpr double eps = 0.000001;

class local_search {
private:
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> g;
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    std::vector<uint32_t> heap, heap_index, dscore, w, edge_w, age;
    std::vector<bool> S, best_S, conf, tabu;

    uint32_t N, E, cost, best_cost, step, previous_top;

    bool compare_nodes(uint32_t u, uint32_t v) {
        if (!conf[u] || tabu[u] || !S[u])
            return false;
        if (!conf[v] || tabu[v] || !S[v])
            return true;
        double us = (double)w[u] / ((double)dscore[u] + eps), vs = (double)w[v] / ((double)dscore[v] + eps);
        return us > vs || (!(vs > us) && age[u] < age[v]);
    }

    uint32_t heap_top() {
        return heap[0];
    }

    void move_up(uint32_t u) {
        uint32_t i = heap_index[u];
        uint32_t p = (i - 1) / 2;
        while (i != 0 && compare_nodes(heap[i], heap[p])) {
            std::swap(heap_index[heap[i]], heap_index[heap[p]]);
            std::swap(heap[i], heap[p]);
            i = p;
            p = (i - 1) / 2;
        }
    }

    void move_down(uint32_t u) {
        uint32_t i = heap_index[u];
        uint32_t l = (i * 2) + 1, r = (i * 2) + 2;
        uint32_t min_child = (r >= N || compare_nodes(heap[l], heap[r])) ? l : r;
        while (min_child < N && compare_nodes(heap[min_child], heap[i])) {
            std::swap(heap_index[heap[i]], heap_index[heap[min_child]]);
            std::swap(heap[i], heap[min_child]);
            i = min_child;
            l = (i * 2) + 1, r = (i * 2) + 2;
            min_child = (r >= N || compare_nodes(heap[l], heap[r])) ? l : r;
        }
    }

    void make_heap() {
        for (uint32_t i = N; i > 0; --i) {
            move_down(heap[i - 1]);
        }
    }

    void update_node_in_heap(uint32_t u) {
        move_up(u);
        move_down(u);
    }

public:
    local_search(uint32_t N, uint32_t E, const std::vector<uint32_t> &weights, const std::vector<std::pair<uint32_t, uint32_t>> &edges, const std::vector<std::optional<bool>> &vc)
        : N(N), E(E), cost(0), best_cost(0), step(0), previous_top(N), g(N), edges(edges), heap(N), heap_index(N), dscore(N, 0), w(N), edge_w(E, 1), age(N, 0), S(N), best_S(N), conf(N, true), tabu(N, false) {
        std::copy(std::begin(weights), std::end(weights), std::begin(w));
        std::iota(std::begin(heap), std::end(heap), 0);
        std::iota(std::begin(heap_index), std::end(heap_index), 0);
        for (uint32_t i = 0; i < E; ++i) {
            auto &&[u, v] = edges[i];
            g[u].push_back({v, i});
            g[v].push_back({u, i});
        }
        for (uint32_t i = 0; i < N; ++i) {
            if (*vc[i]) {
                S[i] = true;
                cost += w[i];
            }
        }
        for (auto &&[u, v] : edges) {
            if (S[u] && !S[v]) {
                dscore[u]++;
            } else if (!S[u] && S[v]) {
                dscore[v]++;
            }
        }
        for (uint32_t u = 0; u < N; ++u) {
            if (S[u] && dscore[u] == 0) {
                S[u] = false;
                cost -= w[u];
                for (auto &&[v, id] : g[u]) {
                    dscore[v]++;
                }
            }
        }
        std::copy(std::begin(S), std::end(S), std::begin(best_S));
        best_cost = cost;
        make_heap();
    }

    bool search(uint32_t iterations) {
        for (uint32_t i = 0; i < iterations; ++i) {
            uint32_t u = heap_top();
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
            std::sort(std::begin(g[u]), std::end(g[u]), [&](auto &&a, auto &&b) {
                return ((double)edge_w[a.second] / (double)w[a.first]) > ((double)edge_w[b.second] / (double)w[b.first]);
            });

            uint32_t count = 1;
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
            std::copy(std::begin(S), std::end(S), std::begin(best_S));
            return true;
        }
        return false;
    }

    uint32_t get_cover(std::vector<std::optional<bool>> &vc) const {
        for (uint32_t i = 0; i < N; ++i) {
            vc[i] = best_S[i];
        }
        return best_cost;
    }

    uint32_t get_best_cost() const {
        return best_cost;
    }
};