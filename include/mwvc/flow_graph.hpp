#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

template <typename Tn, typename Tw>
class flow_graph {
private:
    std::vector<std::pair<Tn, Tw>> edges;
    std::vector<Tn> distance, count;
    std::vector<Tw> excess;
    std::vector<size_t> neighborhood_start;
    std::queue<Tn> excess_v;
    std::vector<bool> active;

    Tn selected_node = 0;
    Tn work_counter = 0;

    void push(Tn u, Tn v);
    void relable(Tn u);
    void global_relable(Tn s, Tn t);
    void gap(Tn level);
    void discharge(Tn u);

public:
    flow_graph(Tn N, const std::vector<std::tuple<Tn, Tn, Tw>> &e);

    size_t size() const;

    Tw solve(Tn s, Tn t);

    flow_graph<Tn, Tw> &
    operator[](Tn u);

    typename std::vector<std::pair<Tn, Tw>>::iterator begin();
    typename std::vector<std::pair<Tn, Tw>>::iterator end();
};

namespace std {
    template <typename Tn, typename Tw>
    typename std::vector<std::pair<Tn, Tw>>::iterator begin(flow_graph<Tn, Tw> &g) { return g.begin(); }

    template <typename Tn, typename Tw>
    typename std::vector<std::pair<Tn, Tw>>::iterator end(flow_graph<Tn, Tw> &g) { return g.end(); }
}

template <typename Tn, typename Tw>
flow_graph<Tn, Tw>::flow_graph(Tn N, const std::vector<std::tuple<Tn, Tn, Tw>> &e)
    : edges(e.size() * 2), distance(N, 0), count(N * 2, 0), excess(N, 0), neighborhood_start(N + 1, 0), excess_v(), active(N, false) {
    for (auto &&[u, v, w] : e) {
        neighborhood_start[u]++;
        neighborhood_start[v]++;
    }
    size_t prefix_sum = 0;
    for (auto &&s : neighborhood_start) {
        prefix_sum += s;
        s = prefix_sum - s;
    }
    std::vector<Tn> d(N, 0);
    for (auto &&[u, v, w] : e) {
        edges[neighborhood_start[u] + d[u]++] = {v, w};
        edges[neighborhood_start[v] + d[v]++] = {u, 0};
    }
    for (Tn u = 0; u < N; u++) {
        std::sort(std::begin(edges) + neighborhood_start[u], std::begin(edges) + neighborhood_start[u + 1]);
    }
}

template <typename Tn, typename Tw>
size_t flow_graph<Tn, Tw>::size() const { return distance.size(); }

template <typename Tn, typename Tw>
flow_graph<Tn, Tw> &flow_graph<Tn, Tw>::operator[](Tn u) {
    assert(u >= 0 && u < size());
    selected_node = u;
    return *this;
}

template <typename Tn, typename Tw>
typename std::vector<std::pair<Tn, Tw>>::iterator flow_graph<Tn, Tw>::begin() {
    assert(selected_node >= 0 && selected_node < size());
    return std::begin(edges) + neighborhood_start[selected_node];
}

template <typename Tn, typename Tw>
typename std::vector<std::pair<Tn, Tw>>::iterator flow_graph<Tn, Tw>::end() {
    assert(selected_node >= 0 && selected_node < size());
    return std::begin(edges) + neighborhood_start[selected_node + 1];
}

template <typename Tn, typename Tw>
void flow_graph<Tn, Tw>::push(Tn u, Tn v) {
    auto &&[_v, uv_c] = *std::lower_bound(std::begin((*this)[u]), std::end((*this)[u]), v, [&](auto &&a, auto &&b) { return a.first < b; });
    auto &&[_u, vu_c] = *std::lower_bound(std::begin((*this)[v]), std::end((*this)[v]), u, [&](auto &&a, auto &&b) { return a.first < b; });
    Tw c = std::min(excess[u], uv_c);
    uv_c -= c;
    vu_c += c;
    excess[u] -= c;
    excess[v] += c;
    if (!active[v] && c > 0 && excess[v] == c) {
        active[v] = true;
        excess_v.push(v);
    }
}

template <typename Tn, typename Tw>
void flow_graph<Tn, Tw>::relable(Tn u) {
    count[distance[u]]--;
    distance[u] = 2 * size();

    work_counter += 10 + std::distance(std::begin((*this)[u]), std::end((*this)[u]));
    for (auto &&[v, c] : (*this)[u]) {
        if (c > 0)
            distance[u] = std::min(distance[u], (Tn)(distance[v] + 1));
    }
    count[distance[u]]++;
    if (!active[u] && excess[u] > 0) {
        active[u] = true;
        excess_v.push(u);
    }
}

template <typename Tn, typename Tw>
void flow_graph<Tn, Tw>::global_relable(Tn s, Tn t) {
    std::queue<Tn> q;
    std::vector<bool> visited(size(), false);
    for (Tn u = 0; u < size(); u++)
        distance[u] = std::max(distance[u], (Tn)size());
    visited[s] = true;
    visited[t] = true;
    q.push(t);
    distance[t] = 0;

    Tn u;
    while (!q.empty()) {
        u = q.front();
        q.pop();

        for (auto &&[v, c] : (*this)[u]) {
            if (visited[v])
                continue;
            auto &&[_u, vu_c] = *std::lower_bound(std::begin((*this)[v]), std::end((*this)[v]), u, [&](auto &&a, auto &&b) { return a.first < b; });
            if (vu_c > 0) {
                count[distance[v]]--;
                distance[v] = distance[u] + 1;
                count[distance[v]]++;
                q.push(v);
                visited[v] = true;
            }
        }
    }
}

template <typename Tn, typename Tw>
void flow_graph<Tn, Tw>::gap(Tn level) {
    for (Tn u = 0; u < size(); u++) {
        if (distance[u] < level)
            continue;
        count[distance[u]]--;
        distance[u] = std::max(distance[u], (Tn)size());
        count[distance[u]]++;
        if (!active[u] && excess[u] > 0) {
            active[u] = true;
            excess_v.push(u);
        }
    }
}

template <typename Tn, typename Tw>
void flow_graph<Tn, Tw>::discharge(Tn u) {
    for (auto &&[v, c] : (*this)[u]) {
        if (c > 0 && distance[u] > distance[v])
            push(u, v);
        if (excess[u] == 0)
            break;
    }
    if (excess[u] > 0) {
        if (count[distance[u]] == 1 && distance[u] < size())
            gap(distance[u]);
        else
            relable(u);
    }
}

template <typename Tn, typename Tw>
Tw flow_graph<Tn, Tw>::solve(Tn s, Tn t) {
    distance[s] = size();
    count[0] = size() - 1;
    count[size()] = 1;
    active[s] = true;
    active[t] = true;

    excess[s] = std::numeric_limits<Tw>::max();
    for (auto &&[u, c] : (*this)[s])
        push(s, u);
    global_relable(s, t);

    while (!excess_v.empty()) {
        Tn u = excess_v.front();
        excess_v.pop();
        active[u] = false;
        if (u != s && u != t)
            discharge(u);

        if (work_counter > (((4 * size()) + edges.size()) / 2)) {
            work_counter = 0;
            global_relable(s, t);
        }
    }

    Tw flow = 0;
    for (auto &&[u, c] : (*this)[t])
        flow += c;
    return flow;
}