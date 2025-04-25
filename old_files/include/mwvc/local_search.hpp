#pragma once
#include "mwvc_reductions.hpp"
#include <algorithm>
#include <numeric>
#include <vector>

class local_search {
private:
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> g;
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    std::vector<uint32_t> heap, heap_index, dscore, w, edge_w, age;
    std::vector<bool> S, best_S, conf;

    uint32_t N, E, cost, best_cost, step, best_seen;

    bool compare_nodes(uint32_t u, uint32_t v) {
        if (!conf[u] || !S[u])
            return false;
        if (!conf[v] || !S[v])
            return true;
        double us = (double)dscore[u] / (double)w[u], vs = (double)dscore[v] / (double)w[v];
        return us < vs || (!(vs < us) && age[u] < age[v]);
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
        : N(N), E(E), cost(0), best_cost(0), step(0), best_seen(std::numeric_limits<uint32_t>::max()), g(N), edges(edges), heap(N), heap_index(N), dscore(N, 0), w(N), edge_w(E, 1), age(N, 0), S(N, false), best_S(N), conf(N, true) {
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

    local_search(vertex_cover<uint32_t, uint32_t> &vc, reduction_graph<uint32_t, uint32_t> &g)
        : N(g.size()), E(0), cost(0), best_cost(0), step(0), best_seen(std::numeric_limits<uint32_t>::max()), g(g.size()), edges(), heap(g.size()), heap_index(g.size()), dscore(g.size(), 0), w(g.size()), edge_w(), age(g.size(), 0), S(g.size(), false), best_S(g.size()), conf(g.size(), true) {
        for (uint32_t i = 0; i < N; ++i) {
            w[i] = g.W(i);
            for (auto &&u : g[i]) {
                if (u > i)
                    edges.push_back({i, u});
            }
        }
        E = edges.size();
        edge_w = std::vector<uint32_t>(E, 1);
        std::iota(std::begin(heap), std::end(heap), 0);
        std::iota(std::begin(heap_index), std::end(heap_index), 0);
        for (uint32_t i = 0; i < E; ++i) {
            auto &&[u, v] = edges[i];
            this->g[u].push_back({v, i});
            this->g[v].push_back({u, i});
        }
        for (uint32_t i = 0; i < N; ++i) {
            if (*vc.S[g.get_org_label(i)]) {
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
                for (auto &&[v, id] : this->g[u]) {
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
            step++;
            uint32_t u = heap_top();
            if (!S[u]) {
                for (uint32_t v = 0; v < N; ++v) {
                    if (S[v] && !conf[v]) {
                        conf[v] = true;
                        update_node_in_heap(v);
                    }
                }
                continue;
            }
            S[u] = false;
            cost -= w[u];
            dscore[u] = 0;
            age[u] = step;
            update_node_in_heap(u);
            auto it = std::partition(std::begin(g[u]), std::end(g[u]), [&](auto &&a) { return !S[a.first]; });
            std::sort(std::begin(g[u]), it, [&](auto &&a, auto &&b) {
                double av = (double)edge_w[a.second] / (double)w[a.first], bv = (double)edge_w[b.second] / (double)w[b.first];
                return av > bv || (!(av < bv) && age[a.first] < age[b.first]);
            });

            uint32_t count = 1;
            for (auto &&[v, id] : g[u]) {
                if (!S[v]) {
                    // conf[v] = false;
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
                        // conf[w] = true;
                        update_node_in_heap(w);
                    }
                    count++;
                } else {
                    // conf[v] = true;
                    dscore[v] += edge_w[id];
                    update_node_in_heap(v);
                }
            }
            best_seen = std::min(best_seen, cost);
        }

        if (cost < best_cost) {
            best_cost = cost;
            std::copy(std::begin(S), std::end(S), std::begin(best_S));
            return true;
        }
        return false;
    }

    uint32_t get_cover(vertex_cover<uint32_t, uint32_t> &vc, const reduction_graph<uint32_t, uint32_t> &g) const {
        for (uint32_t i = 0; i < N; ++i) {
            if (*vc.S[g.get_org_label(i)] && !best_S[i]) {
                vc.cost -= g.W(i);
            } else if (!*vc.S[g.get_org_label(i)] && best_S[i]) {
                vc.cost += g.W(i);
            }
            vc.S[g.get_org_label(i)] = best_S[i];
        }
        return best_cost;
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

    uint32_t get_cost() const {
        return cost;
    }

    uint32_t get_best_seen() const {
        return best_seen;
    }
};