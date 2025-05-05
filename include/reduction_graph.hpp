#pragma once
#include <algorithm>
#include <cassert>
#include <numeric>
#include <stack>
#include <vector>

/*
    Undirected graph structure for the VC and IS problems.

    Notes:
        * Begin/end behaviour depends on g[u] being called before. Intended use: begin(g[u]) or "for (auto &&v : g[u])"
*/

enum class action
{
    node_remove,         // [a, u, _]
    neighborhood_remove, // [a, u, _]
    neighborhood_fold,   // [a, u, new_node]
    twin_fold,           // [a, u, v] (v folds into u)
    isolated_fold,       // [a, u, _]
    relable_graph        // [a, N, _]
};

template <typename Tn, typename Tw>
class reduction_graph
{
private:
    std::vector<bool> active;
    std::vector<Tn> edges, org_label, new_label;
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

    void undo_relable_graph(Tn n);

public:
    reduction_graph(const std::vector<Tw> &w, const std::vector<std::pair<Tn, Tn>> &e);

    Tn get_timestamp() const;
    Tn get_org_label(Tn u) const;
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

    void relable_graph();

    std::tuple<action, Tn, Tn> actions_top() const;
    void actions_pop();

    typename std::vector<Tn>::const_iterator begin() const;
    typename std::vector<Tn>::const_iterator end() const;

    typename std::vector<Tn>::const_iterator begin(Tn u) const;
    typename std::vector<Tn>::const_iterator end(Tn u) const;
};

template <typename Tn, typename Tw>
typename std::vector<Tn>::iterator reduction_graph<Tn, Tw>::Nf(Tn u)
{
    assert(u >= 0 && u < N);
    return std::begin(edges) + neighborhood_range[u].first;
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::iterator reduction_graph<Tn, Tw>::Nl(Tn u)
{
    assert(u >= 0 && u < N);
    return std::begin(edges) + neighborhood_range[u].second;
}

template <typename Tn, typename Tw>
reduction_graph<Tn, Tw>::reduction_graph(const std::vector<Tw> &w, const std::vector<std::pair<Tn, Tn>> &e)
    : active(w.size(), true), edges(e.size() * 2), org_label(w.size()), new_label(w.size()), node_weights(w), neighborhood_weights(w.size(), 0), neighborhood_range(w.size(), {0, 0}), N(w.size())
{
    assert(std::is_sorted(std::begin(e), std::end(e)));
    std::iota(std::begin(org_label), std::end(org_label), 0);
    for (auto &&[u, v] : e)
    {
        neighborhood_range[u].second++;
        neighborhood_range[v].second++;
        neighborhood_weights[u] += w[v];
        neighborhood_weights[v] += w[u];
    }
    size_t prefix_sum = 0;
    for (auto &&[s, t] : neighborhood_range)
    {
        prefix_sum += t;
        s = prefix_sum - t;
        t = s;
    }
    for (auto &&[u, v] : e)
    {
        edges[neighborhood_range[u].second++] = v;
        edges[neighborhood_range[v].second++] = u;
    }
}

template <typename Tn, typename Tw>
Tn reduction_graph<Tn, Tw>::get_timestamp() const { return action_log.size(); }

template <typename Tn, typename Tw>
Tn reduction_graph<Tn, Tw>::get_org_label(Tn u) const
{
    assert(u >= 0 && u < org_label.size());
    return org_label[u];
}

template <typename Tn, typename Tw>
Tn reduction_graph<Tn, Tw>::size() const { return N; }

template <typename Tn, typename Tw>
Tn reduction_graph<Tn, Tw>::D(Tn u) const { return std::distance(std::begin(this->operator[](u)), std::end(this->operator[](u))); }

template <typename Tn, typename Tw>
Tw reduction_graph<Tn, Tw>::W(Tn u) const
{
    assert(u >= 0 && u < N);
    return node_weights[u];
}

template <typename Tn, typename Tw>
Tw reduction_graph<Tn, Tw>::NW(Tn u) const
{
    assert(u >= 0 && u < N);
    return neighborhood_weights[u];
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_equal_to(const reduction_graph<Tn, Tw> &g) const
{
    return N == g.N && action_log.size() == g.action_log.size() &&
           std::equal(std::begin(edges), std::end(edges), std::begin(g.edges), std::end(g.edges)) &&
           std::equal(std::begin(node_weights), std::end(node_weights), std::begin(g.node_weights), std::end(g.node_weights)) &&
           std::equal(std::begin(neighborhood_weights), std::end(neighborhood_weights), std::begin(g.neighborhood_weights), std::end(g.neighborhood_weights)) &&
           std::equal(std::begin(neighborhood_range), std::end(neighborhood_range), std::begin(g.neighborhood_range), std::end(g.neighborhood_range)) &&
           std::equal(std::begin(org_label), std::end(org_label), std::begin(g.org_label), std::end(g.org_label));
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_active(Tn u) const
{
    assert(u >= 0 && u < N);
    return active[u];
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_twin(Tn u, Tn v) const
{
    assert(u >= 0 && u < N && v >= 0 && v < N);
    if (D(u) != D(v) || NW(u) != NW(v) || u == v)
        return false;
    return std::equal(std::begin(this->operator[](u)), std::end(this->operator[](u)), std::begin(this->operator[](v)));
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_isolated(Tn u) const
{
    assert(u >= 0 && u < N);
    return std::all_of(std::begin(this->operator[](u)), std::end(this->operator[](u)), [&](Tn v)
                       {
        if (is_dominating(v, u)) {
            assert(W(v) > W(u));
            return true;
        }
        return false; });
}

template <typename Tn, typename Tw>
bool reduction_graph<Tn, Tw>::is_dominating(Tn u, Tn v) const
{
    assert(u >= 0 && u < N && v >= 0 && v < N);
    if (D(u) < D(v) || (W(u) + NW(u)) < (W(v) + NW(v)))
        return false;
    auto &&f1 = std::begin(this->operator[](u)), l1 = std::end(this->operator[](u));
    auto &&f2 = std::begin(this->operator[](v)), l2 = std::end(this->operator[](v));
    while (f2 != l2)
    {
        if (*f2 == u)
        {
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
bool reduction_graph<Tn, Tw>::has_independent_neighbors(Tn u) const
{
    assert(u >= 0 && u < N);
    std::vector<Tn> tmp;
    for (auto &&v : this->operator[](u))
    {
        std::set_intersection(std::begin(this->operator[](u)), std::end(this->operator[](u)), std::begin(this->operator[](v)), std::end(this->operator[](v)), std::back_inserter(tmp));
        if (!tmp.empty())
            return false;
    }
    return true;
}

template <typename Tn, typename Tw>
const reduction_graph<Tn, Tw> &reduction_graph<Tn, Tw>::operator[](Tn u) const
{
    assert(u >= 0 && u < N);
    selected_node = u;
    return *this;
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::remove_node(Tn u)
{
    assert(u >= 0 && u < N && active[u]);

    active[u] = false;
    action_log.push({action::node_remove, u, 0});

    for (auto &&v : this->operator[](u))
    {
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        assert(*it == u);
        std::rotate(Nf(v), it, it + 1);
        neighborhood_range[v].first++;
        neighborhood_weights[v] -= W(u);
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_remove_node(Tn u)
{
    assert(u >= 0 && u < N && !active[u]);

    active[u] = true;
    for (auto &&v : this->operator[](u))
    {
        assert(*(Nf(v) - 1) == u);
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        neighborhood_range[v].first--;
        std::rotate(Nf(v), Nf(v) + 1, it);
        neighborhood_weights[v] += W(u);
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::remove_neighborhood(Tn u)
{
    assert(u >= 0 && u < N && active[u]);

    active[u] = false;
    action_log.push({action::neighborhood_remove, u, 0});

    for (auto &&v : this->operator[](u))
    {
        active[v] = false;
    }

    for (auto &&v : this->operator[](u))
    {
        for (auto &&w : this->operator[](v))
        {
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
void reduction_graph<Tn, Tw>::undo_remove_neighborhood(Tn u)
{
    assert(u >= 0 && u < N && !active[u]);

    for (auto &&v : this->operator[](u))
    {
        for (auto &&w : this->operator[](v))
        {
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
void reduction_graph<Tn, Tw>::fold_neighborhood(Tn u)
{
    assert(u >= 0 && u < N && active[u]);

    active.push_back(true);
    node_weights.push_back(NW(u) - W(u));
    neighborhood_weights.push_back(0);
    neighborhood_range.push_back({edges.size(), edges.size()});
    org_label.push_back(org_label.size());
    new_label.push_back(0);

    if (N != active.size() - 1)
    {
        // std::swap(active.back(), active[N]);
        active.swap(active.back(), active[N]);
        std::swap(neighborhood_range.back(), neighborhood_range[N]);
        std::swap(node_weights.back(), node_weights[N]);
        std::swap(neighborhood_weights.back(), neighborhood_weights[N]);
        std::swap(org_label.back(), org_label[N]);
    }

    active[u] = false;
    action_log.push({action::neighborhood_fold, u, N});

    for (auto &&v : this->operator[](u))
    {
        active[v] = false;
    }

    std::vector<Tn> tmp;

    for (auto &&v : this->operator[](u))
    {
        for (auto &&w : this->operator[](v))
        {
            if (!active[w])
                continue;

            auto &&it = std::lower_bound(Nf(w), Nl(w), v);
            assert(*it == v);
            neighborhood_weights[w] -= W(v);
            if (*(Nl(w) - 1) == N)
            { // already changed
                std::rotate(Nf(w), it, it + 1);
                neighborhood_range[w].first++;
            }
            else
            {
                *it = N;
                if (it + 1 != Nl(w))
                    std::rotate(it, it + 1, Nl(w));
                tmp.push_back(w);
                neighborhood_range[N].second++;
                neighborhood_weights[N] += W(w);
                neighborhood_weights[w] += node_weights[N];
            }
        }
    }

    N++;
    std::sort(std::begin(tmp), std::end(tmp));
    edges.insert(std::end(edges), std::begin(tmp), std::end(tmp));
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_fold_neighborhood(Tn u)
{
    assert(u >= 0 && u < N && !active[u]);

    for (auto &&v : this->operator[](u))
    {
        for (auto &&w : this->operator[](v))
        {
            if (!active[w])
                continue;

            neighborhood_weights[w] += W(v);
            if (*(Nl(w) - 1) == N - 1)
            {
                auto &&it = std::lower_bound(Nf(w), Nl(w), v);
                *(Nl(w) - 1) = v;
                std::rotate(it, Nl(w) - 1, Nl(w));
                neighborhood_weights[w] -= node_weights[N - 1];
            }
            else
            {
                neighborhood_range[w].first--;
                auto &&it = std::lower_bound(Nf(w) + 1, Nl(w), *Nf(w));
                std::rotate(Nf(w), Nf(w) + 1, it);
            }
        }
    }

    active[u] = true;

    for (auto &&v : this->operator[](u))
        active[v] = true;

    edges.erase(Nf(N - 1), Nl(N - 1));

    if (N != active.size())
    {
        // std::swap(active.back(), active[N - 1]);
        active.swap(active.back(), active[N - 1]);
        std::swap(neighborhood_range.back(), neighborhood_range[N - 1]);
        std::swap(node_weights.back(), node_weights[N - 1]);
        std::swap(neighborhood_weights.back(), neighborhood_weights[N - 1]);
        std::swap(org_label.back(), org_label[N - 1]);
    }

    N--;
    active.pop_back();
    node_weights.pop_back();
    neighborhood_weights.pop_back();
    neighborhood_range.pop_back();
    org_label.pop_back();
    new_label.pop_back();
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::fold_twin(Tn u, Tn v)
{
    assert(u >= 0 && u < N && active[u] && v >= 0 && v < N && active[v]);

    active[v] = false;
    action_log.push({action::twin_fold, u, v});

    for (auto &&w : this->operator[](v))
    {
        auto &&it = std::lower_bound(Nf(w), Nl(w), v);
        assert(*it == v);
        std::rotate(Nf(w), it, it + 1);
        neighborhood_range[w].first++;
    }
    node_weights[u] += W(v);
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_fold_twin(Tn u, Tn v)
{
    assert(u >= 0 && u < N && active[u] && v >= 0 && v < N && !active[v]);

    active[v] = true;
    for (auto &&w : this->operator[](v))
    {
        assert(*(Nf(w) - 1) == v);
        auto &&it = std::lower_bound(Nf(w), Nl(w), v);
        neighborhood_range[w].first--;
        std::rotate(Nf(w), Nf(w) + 1, it);
    }
    node_weights[u] -= W(v);
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::fold_isolated(Tn u)
{
    assert(u >= 0 && u < N && active[u]);

    active[u] = false;
    action_log.push({action::isolated_fold, u, 0});

    for (auto &&v : this->operator[](u))
    {
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        assert(*it == u);
        std::rotate(Nf(v), it, it + 1);
        neighborhood_range[v].first++;
        neighborhood_weights[v] -= W(u);
        node_weights[v] -= W(u);
        for (auto &&w : this->operator[](v))
        {
            if (w != u)
                neighborhood_weights[w] -= W(u);
        }
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_fold_isolated(Tn u)
{
    assert(u >= 0 && u < N && !active[u]);

    active[u] = true;

    for (auto &&v : this->operator[](u))
    {
        assert(*(Nf(v) - 1) == u);
        auto &&it = std::lower_bound(Nf(v), Nl(v), u);
        neighborhood_range[v].first--;
        std::rotate(Nf(v), Nf(v) + 1, it);

        neighborhood_weights[v] += W(u);
        node_weights[v] += W(u);
        for (auto &&w : this->operator[](v))
        {
            if (w != u)
                neighborhood_weights[w] += W(u);
        }
    }
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::relable_graph()
{
    auto it_active = std::find(std::begin(active), std::begin(active) + N, true);
    auto it_deactive = std::find(std::begin(active), std::begin(active) + N, false);
    if (it_deactive == std::begin(active) + N)
        return;

    action_log.push({action::relable_graph, N, 0});

    std::iota(std::begin(new_label), std::begin(new_label) + N, 0);

    while (it_active != std::begin(active) + N && it_deactive != std::begin(active) + N)
    {
        if (std::distance(it_active, it_deactive) < 0)
        {
            // swap
            size_t i = std::distance(std::begin(active), it_active), j = std::distance(std::begin(active), it_deactive);

            // std::swap(active[i], active[j]);
            active.swap(active[i], active[j]);
            std::swap(neighborhood_range[i], neighborhood_range[j]);
            std::swap(node_weights[i], node_weights[j]);
            std::swap(neighborhood_weights[i], neighborhood_weights[j]);
            std::swap(org_label[i], org_label[j]);
            new_label[i] = j;

            while (it_active != std::begin(active) + N && !(*it_active))
                ++it_active;
            while (it_deactive != std::begin(active) + N && *it_deactive)
                ++it_deactive;
        }
        else
        {
            do
            {
                ++it_active;
            } while (it_active != std::begin(active) + N && !(*it_active));
        }
    }
    size_t new_N = 0;
    for (size_t u = 0; u < N; ++u)
    {
        if (!active[u])
            continue;

        ++new_N;
        std::transform(Nf(u), Nl(u), Nf(u), [&](size_t v)
                       { return new_label[v]; });
    }
    N = new_N;
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::undo_relable_graph(Tn n)
{
    N = n;
    auto it_active = std::find(std::rbegin(active) + (active.size() - N), std::rend(active), true);
    auto it_deactive = std::find(std::rbegin(active) + (active.size() - N), std::rend(active), false);

    std::iota(std::begin(new_label), std::begin(new_label) + N, 0);

    while (it_active != std::rend(active) && it_deactive != std::rend(active))
    {

        size_t i = std::distance(it_active, std::rend(active)) - 1, j = std::distance(it_deactive, std::rend(active)) - 1;

        if (org_label[i] > org_label[j])
        {
            // swap
            // for (auto &&v : this->operator[](i)) {
            //     auto &&u = std::lower_bound(Nf(v), Nl(v), i);
            //     *u = j;
            // }
            // std::swap(active[i], active[j]);
            active.swap(active[i], active[j]);
            std::swap(neighborhood_range[i], neighborhood_range[j]);
            std::swap(node_weights[i], node_weights[j]);
            std::swap(neighborhood_weights[i], neighborhood_weights[j]);
            std::swap(org_label[i], org_label[j]);
            new_label[i] = j;

            while (it_active != std::rend(active) && !(*it_active))
                ++it_active;
            while (it_deactive != std::rend(active) && *it_deactive)
                ++it_deactive;
        }
        else
        {
            do
            {
                ++it_deactive;
            } while (it_deactive != std::rend(active) && *it_deactive);
        }
    }

    for (size_t u = 0; u < N; ++u)
    {
        if (!active[u])
            continue;
        std::transform(Nf(u), Nl(u), Nf(u), [&](size_t v)
                       { return new_label[v]; });
    }
}

template <typename Tn, typename Tw>
std::tuple<action, Tn, Tn> reduction_graph<Tn, Tw>::actions_top() const
{
    return action_log.top();
}

template <typename Tn, typename Tw>
void reduction_graph<Tn, Tw>::actions_pop()
{
    auto &&[t, u, v] = action_log.top();
    action_log.pop();
    switch (t)
    {
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
    case action::relable_graph:
        undo_relable_graph(u);
        break;

    default:
        break;
    }
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::const_iterator reduction_graph<Tn, Tw>::begin() const
{
    assert(selected_node >= 0 && selected_node < N);
    return std::begin(edges) + neighborhood_range[selected_node].first;
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::const_iterator reduction_graph<Tn, Tw>::end() const
{
    assert(selected_node >= 0 && selected_node < N);
    return std::begin(edges) + neighborhood_range[selected_node].second;
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::const_iterator reduction_graph<Tn, Tw>::begin(Tn u) const
{
    assert(u >= 0 && u < N);
    return std::begin(edges) + neighborhood_range[u].first;
}

template <typename Tn, typename Tw>
typename std::vector<Tn>::const_iterator reduction_graph<Tn, Tw>::end(Tn u) const
{
    assert(u >= 0 && u < N);
    return std::begin(edges) + neighborhood_range[u].second;
}