#pragma once
#include <algorithm>
#include <cassert>
#include <stack>
#include <vector>

/*
    Undirected graph structure for the VC and IS problems.

    Notes:
        * Begin/end behaviour depends on g[u] being called before. Intended use: begin(g[u]) or "for (auto &&v : g[u])"
*/

enum class action { node_remove,         // [a, u, _]
                    neighborhood_remove, // [a, u, _]
                    neighborhood_fold,   // [a, u, new_node]
                    twin_fold,           // [a, u, v] (v folds into u)
                    isolated_fold };     // [a, u, _]

template <typename Tn, typename Tw>
class reduction_graph {
private:
    std::vector<bool> active;
    std::vector<Tn> edges;
    std::vector<Tw> node_weights, neighborhood_weights;
    std::vector<std::pair<size_t, size_t>> neighborhood_range;
    std::stack<std::tuple<action, Tn, Tn>> action_log;

    Tn N = 0;
    mutable Tn selected_node = 0;

    typename std::vector<Tn>::iterator Nf(Tn u);
    typename std::vector<Tn>::iterator Nl(Tn u);

    void undo_remove_node(Tn u);
    void undo_remove_neighborhood(Tn u);

    void undo_fold_neighborhood(Tn u);
    void undo_fold_twin(Tn u, Tn v);
    void undo_fold_isolated(Tn u);

public:
    reduction_graph(const std::vector<Tw> &w, const std::vector<std::pair<Tn, Tn>> &e);

    Tn get_timestamp() const;
    Tn size() const;
    Tn D(Tn u) const;

    Tw W(Tn u) const;
    Tw NW(Tn u) const;

    bool is_equal_to(const reduction_graph<Tn, Tw> &g) const;
    bool is_active(Tn u) const;
    bool is_twin(Tn u, Tn v) const;
    bool is_isolated(Tn u) const;
    bool is_dominating(Tn u, Tn v) const;
    bool has_independent_neighbors(Tn u) const;

    const reduction_graph<Tn, Tw> &operator[](Tn u) const;

    void remove_node(Tn u);
    void remove_neighborhood(Tn u);

    void fold_neighborhood(Tn u);
    void fold_twin(Tn u, Tn v);
    void fold_isolated(Tn u);

    std::tuple<action, Tn, Tn> actions_top() const;
    void actions_pop();

    typename std::vector<Tn>::const_iterator begin() const;
    typename std::vector<Tn>::const_iterator end() const;
};

template <typename Tn, typename Tw>
typename std::vector<Tn>::iterator reduction_graph<Tn, Tw>::Nf(Tn u) {
    assert(u >= 0 && u < N);
    return std::begin(edges) + neighborhood_range[u].first;
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::iterator reduction_graph<Tn, Tw>::Nl(Tn u) {
    assert(u >= 0 && u < N);
    return std::begin(edges) + neighborhood_range[u].second;
}

template <typename Tn, typename Tw>
reduction_graph<Tn, Tw>::reduction_graph(const std::vector<Tw> &w, const std::vector<std::pair<Tn, Tn>> &e)
    : active(w.size(), true), edges(e.size() * 2), node_weights(w), neighborhood_weights(w.size(), 0), neighborhood_range(w.size(), {0, 0}), N(w.size()) {
    assert(std::is_sorted(std::begin(e), std::end(e)));
    for (auto &&[u, v] : e) {
        neighborhood_range[u].second++;
        neighborhood_range[v].second++;
        neighborhood_weights[u] += w[v];
        neighborhood_weights[v] += w[u];
    }
    size_t prefix_sum = 0;
    for (auto &&[s, t] : neighborhood_range) {
        prefix_sum += t;
        s = prefix_sum - t;
        t = s;
    }
    for (auto &&[u, v] : e) {
        edges[neighborhood_range[u].second++] = v;
        edges[neighborhood_range[v].second++] = u;
    }
}

template <typename Tn, typename Tw>
Tn reduction_graph<Tn, Tw>::get_timestamp() const { return action_log.size(); }

template <typename Tn, typename Tw>
Tn reduction_graph<Tn, Tw>::size() const { return N; }

template <typename Tn, typename Tw>
Tn reduction_graph<Tn, Tw>::D(Tn u) const { return std::distance(std::begin(this->operator[](u)), std::end(this->operator[](u))); }

template <typename Tn, typename Tw>
Tw reduction_graph<Tn, Tw>::W(Tn u) const {
    assert(u >= 0 && u < N);
    return node_weights[u];
}

template <typename Tn, typename Tw>
Tw reduction_graph<Tn, Tw>::NW(Tn u) const {
    assert(u >= 0 && u < N);
    return neighborhood_weights[u];
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_equal_to(const reduction_graph<Tn, Tw> &g) const {
    return N == g.N && action_log.size() == g.action_log.size() &&
           std::equal(std::begin(edges), std::end(edges), std::begin(g.edges), std::end(g.edges)) &&
           std::equal(std::begin(node_weights), std::end(node_weights), std::begin(g.node_weights), std::end(g.node_weights)) &&
           std::equal(std::begin(neighborhood_weights), std::end(neighborhood_weights), std::begin(g.neighborhood_weights), std::end(g.neighborhood_weights)) &&
           std::equal(std::begin(neighborhood_range), std::end(neighborhood_range), std::begin(g.neighborhood_range), std::end(g.neighborhood_range));
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_active(Tn u) const {
    assert(u >= 0 && u < N);
    return active[u];
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_twin(Tn u, Tn v) const {
    assert(u >= 0 && u < N && v >= 0 && v < N);
    if (D(u) != D(v) || NW(u) != NW(v) || u == v)
        return false;
    return std::equal(std::begin(this->operator[](u)), std::end(this->operator[](u)), std::begin(this->operator[](v)));
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_isolated(Tn u) const {
    assert(u >= 0 && u < N);
    return std::all_of(std::begin(this->operator[](u)), std::end(this->operator[](u)), [&](Tn v) {
        if (is_dominating(v, u) && W(v) > W(u)) {
            return true;
        }
        return false;
    });
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_dominating(Tn u, Tn v) const {
    assert(u >= 0 && u < N && v >= 0 && v < N);
    if (D(u) < D(v) || (W(u) + NW(u)) < (W(v) + NW(v)))
        return false;
    auto &&f1 = std::begin(this->operator[](u)), l1 = std::end(this->operator[](u));
    auto &&f2 = std::begin(this->operator[](v)), l2 = std::end(this->operator[](v));
    while (f2 != l2) {
        if (*f2 == u) {
            ++f2;
            if (f2 == l2)
                break;
        }
        if (f1 == l1 || *f2 < *f1)
            return false;
        if (!(*f1 < *f2))
            ++f2;
        ++f1;
    }
    return true;
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::has_independent_neighbors(Tn u) const {
    assert(u >= 0 && u < N);
    std::vector<Tn> tmp;
    for (auto &&v : this->operator[](u)) {
        std::set_intersection(std::begin(this->operator[](u)), std::end(this->operator[](u)), std::begin(this->operator[](v)), std::end(this->operator[](v)), std::back_inserter(tmp));
        if (!tmp.empty())
            return false;
    }
    return true;
}

template <typename Tn, typename Tw>
const reduction_graph<Tn, Tw> &reduction_graph<Tn, Tw>::operator[](Tn u) const {
    assert(u >= 0 && u < N);
    selected_node = u;
    return *this;
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::remove_node(Tn u) {
    assert(u >= 0 && u < N && active[u]);

    active[u] = false;
    action_log.push({action::node_remove, u, 0});

    for (auto &&v : this->operator[](u)) {
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        assert(*it == u);
        std::rotate(Nf(v), it, it + 1);
        neighborhood_range[v].first++;
        neighborhood_weights[v] -= W(u);
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_remove_node(Tn u) {
    assert(u >= 0 && u < N && !active[u]);

    active[u] = true;
    for (auto &&v : this->operator[](u)) {
        assert(*(Nf(v) - 1) == u);
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        neighborhood_range[v].first--;
        std::rotate(Nf(v), Nf(v) + 1, it);
        neighborhood_weights[v] += W(u);
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::remove_neighborhood(Tn u) {
    assert(u >= 0 && u < N && active[u]);

    active[u] = false;
    action_log.push({action::neighborhood_remove, u, 0});

    for (auto &&v : this->operator[](u)) {
        active[v] = false;
    }

    for (auto &&v : this->operator[](u)) {
        for (auto &&w : this->operator[](v)) {
            if (!active[w])
                continue;
            auto &&it = std::lower_bound(Nf(w), Nl(w), v);
            assert(*it == v);
            std::rotate(Nf(w), it, it + 1);
            neighborhood_range[w].first++;
            neighborhood_weights[w] -= W(v);
        }
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_remove_neighborhood(Tn u) {
    assert(u >= 0 && u < N && !active[u]);

    for (auto &&v : this->operator[](u)) {
        for (auto &&w : this->operator[](v)) {
            if (!active[w])
                continue;
            neighborhood_range[w].first--;
            auto &&it = std::lower_bound(Nf(w) + 1, Nl(w), *Nf(w));
            if (Nf(w) + 1 != Nl(w))
                std::rotate(Nf(w), Nf(w) + 1, it);
            neighborhood_weights[w] += W(v);
        }
    }

    active[u] = true;

    for (auto &&v : this->operator[](u))
        active[v] = true;
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::fold_neighborhood(Tn u) {
    assert(u >= 0 && u < N && active[u]);

    active.push_back(true);
    node_weights.push_back(NW(u) - W(u));
    neighborhood_weights.push_back(0);
    neighborhood_range.push_back({edges.size(), edges.size()});

    active[u] = false;
    action_log.push({action::neighborhood_fold, u, N});

    for (auto &&v : this->operator[](u)) {
        active[v] = false;
    }

    std::vector<Tn> tmp;

    for (auto &&v : this->operator[](u)) {
        for (auto &&w : this->operator[](v)) {
            if (!active[w])
                continue;

            auto &&it = std::lower_bound(Nf(w), Nl(w), v);
            assert(*it == v);
            neighborhood_weights[w] -= W(v);
            if (*(Nl(w) - 1) == N) { // already changed
                std::rotate(Nf(w), it, it + 1);
                neighborhood_range[w].first++;
            } else {
                *it = N;
                if (it + 1 != Nl(w))
                    std::rotate(it, it + 1, Nl(w));
                tmp.push_back(w);
                neighborhood_range.back().second++;
                neighborhood_weights.back() += W(w);
                neighborhood_weights[w] += node_weights.back();
            }
        }
    }
    N++;
    std::sort(std::begin(tmp), std::end(tmp));
    edges.insert(std::end(edges), std::begin(tmp), std::end(tmp));
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_fold_neighborhood(Tn u) {
    assert(u >= 0 && u < N && !active[u]);

    for (auto &&v : this->operator[](u)) {
        for (auto &&w : this->operator[](v)) {
            if (!active[w])
                continue;

            neighborhood_weights[w] += W(v);
            if (*(Nl(w) - 1) == N - 1) {
                auto &&it = std::lower_bound(Nf(w), Nl(w), v);
                *(Nl(w) - 1) = v;
                std::rotate(it, Nl(w) - 1, Nl(w));
                neighborhood_weights[w] -= node_weights.back();
            } else {
                neighborhood_range[w].first--;
                auto &&it = std::lower_bound(Nf(w) + 1, Nl(w), *Nf(w));
                std::rotate(Nf(w), Nf(w) + 1, it);
            }
        }
    }

    active[u] = true;

    for (auto &&v : this->operator[](u))
        active[v] = true;

    edges.erase(Nf(active.size() - 1), Nl(active.size() - 1));
    N--;
    active.pop_back();
    node_weights.pop_back();
    neighborhood_weights.pop_back();
    neighborhood_range.pop_back();
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::fold_twin(Tn u, Tn v) {
    assert(u >= 0 && u < N && active[u] && v >= 0 && v < N && active[v]);

    active[v] = false;
    action_log.push({action::twin_fold, u, v});

    for (auto &&w : this->operator[](v)) {
        auto &&it = std::lower_bound(Nf(w), Nl(w), v);
        assert(*it == v);
        std::rotate(Nf(w), it, it + 1);
        neighborhood_range[w].first++;
    }
    node_weights[u] += W(v);
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_fold_twin(Tn u, Tn v) {
    assert(u >= 0 && u < N && active[u] && v >= 0 && v < N && !active[v]);

    active[v] = true;
    for (auto &&w : this->operator[](v)) {
        assert(*(Nf(w) - 1) == v);
        auto &&it = std::lower_bound(Nf(w), Nl(w), v);
        neighborhood_range[w].first--;
        std::rotate(Nf(w), Nf(w) + 1, it);
    }
    node_weights[u] -= W(v);
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::fold_isolated(Tn u) {
    assert(u >= 0 && u < N && active[u]);

    active[u] = false;
    action_log.push({action::isolated_fold, u, 0});

    for (auto &&v : this->operator[](u)) {
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        assert(*it == u);
        std::rotate(Nf(v), it, it + 1);
        neighborhood_range[v].first++;
        neighborhood_weights[v] -= W(u);
        node_weights[v] -= W(u);
        for (auto &&w : this->operator[](v)) {
            if (w != u)
                neighborhood_weights[w] -= W(u);
        }
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_fold_isolated(Tn u) {
    assert(u >= 0 && u < N && !active[u]);

    active[u] = true;

    for (auto &&v : this->operator[](u)) {
        assert(*(Nf(v) - 1) == u);
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        neighborhood_range[v].first--;
        std::rotate(Nf(v), Nf(v) + 1, it);

        neighborhood_weights[v] += W(u);
        node_weights[v] += W(u);
        for (auto &&w : this->operator[](v)) {
            if (w != u)
                neighborhood_weights[w] += W(u);
        }
    }
}

template <typename Tn, typename Tw>
std::tuple<action, Tn, Tn> reduction_graph<Tn, Tw>::actions_top() const {
    return action_log.top();
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::actions_pop() {
    auto &&[t, u, v] = action_log.top();
    action_log.pop();
    switch (t) {
    case action::node_remove:
        undo_remove_node(u);
        break;
    case action::neighborhood_remove:
        undo_remove_neighborhood(u);
        break;
    case action::neighborhood_fold:
        undo_fold_neighborhood(u);
        break;
    case action::twin_fold:
        undo_fold_twin(u, v);
        break;
    case action::isolated_fold:
        undo_fold_isolated(u);
        break;

    default:
        break;
    }
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::const_iterator reduction_graph<Tn, Tw>::begin() const {
    assert(selected_node >= 0 && selected_node < N);
    return std::begin(edges) + neighborhood_range[selected_node].first;
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::const_iterator reduction_graph<Tn, Tw>::end() const {
    assert(selected_node >= 0 && selected_node < N);
    return std::begin(edges) + neighborhood_range[selected_node].second;
}