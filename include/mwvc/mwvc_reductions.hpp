#pragma once
#include "flow_graph.hpp"
#include "reduction_graph.hpp"
#include "small_solve.hpp"
#include <bitset>
#include <optional>

template <typename Tn, typename Tw>
struct vertex_cover {
    std::vector<std::optional<bool>> S;
    Tw cost = 0;
    size_t r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    vertex_cover(Tn N) : S(N, std::nullopt) {}

    void extend_to_new_node() {
        S.push_back(std::nullopt);
    }
};

constexpr size_t max_small_solve = 8;

enum class reduction_rules { neighborhood_reduction,
                             twin_fold,
                             domination_reduction,
                             isolated_fold,
                             independent_fold,
                             neighbor_meta_reduction,
                             neighborhood_meta_reduction,
                             critical_weight_reduction
};

template <typename Tn>
struct graph_search {
    std::vector<std::vector<bool>> visited;
    std::vector<std::stack<Tn>> search;
    small_mwvc_solver sms;

    size_t num_local_reduction_rules = 7;

    size_t label_count = 0;

    graph_search(Tn N, size_t rules = 7)
        : visited(rules, std::vector<bool>(N, false)), search(rules), num_local_reduction_rules(rules) {
        for (size_t r = 0; r < num_local_reduction_rules; r++)
            for (Tn u = 0; u < N; u++)
                search[r].push(u);
    }

    void push_search(Tn u) {
        assert(u >= 0 && u < visited[0].size());
        for (size_t r = 0; r < num_local_reduction_rules; r++) {
            if (visited[r][u])
                search[r].push(u);
            visited[r][u] = false;
        }
    }

    Tn pop_search(size_t rule) {
        Tn u = search[rule].top();
        search[rule].pop();
        visited[rule][u] = true;
        return u;
    }

    void extend_to_new_node(Tn u) {
        for (size_t r = 0; r < num_local_reduction_rules; r++) {
            visited[r].push_back(false);
            search[r].push(u);
        }
    }
};

template <typename Tn, typename Tw>
void unfold_graph(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, size_t t) {
    while (g.get_timestamp() > t) {
        auto [t, u, v] = g.actions_top();
        if (t == action::twin_fold) {
            assert(vc.S[g.get_org_label(u)].has_value() && !vc.S[g.get_org_label(v)].has_value());
            vc.S[g.get_org_label(v)] = vc.S[g.get_org_label(u)];
        } else if (t == action::isolated_fold) {
            assert(!vc.S[g.get_org_label(u)].has_value());
            vc.S[g.get_org_label(u)] = std::any_of(std::begin(g[u]), std::end(g[u]), [&](auto w) { return !*vc.S[g.get_org_label(w)]; });
        } else if (t == action::neighborhood_fold) {
            assert(g.get_org_label(v) == vc.S.size() - 1 && vc.S[g.get_org_label(v)].has_value());
            vc.S[g.get_org_label(u)] = !*vc.S[g.get_org_label(v)];
            for (auto &&w : g[u]) {
                vc.S[g.get_org_label(w)] = *vc.S[g.get_org_label(v)];
            }
            vc.S.pop_back();
            for (size_t r = 0; r < gs.num_local_reduction_rules; r++) {
                gs.visited[r].pop_back();
            }
        }
        g.actions_pop();
    }
}

template <typename Tn, typename Tw>
void select_neighborhood(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    Tn u_org = g.get_org_label(u), v_org;
    assert(vc.S[u_org] == std::nullopt);
    vc.S[u_org] = false;
    gs.label_count += g.D(u);
    for (auto &&v : g[u]) {
        v_org = g.get_org_label(v);
        assert(vc.S[v_org] == std::nullopt);
        vc.S[v_org] = true;
        vc.cost += g.W(v);
    }

    g.remove_neighborhood(u);

    for (auto &&v : g[u])
        for (auto &&w : g[v])
            if (g.is_active(w))
                gs.push_search(w);
}

template <typename Tn, typename Tw>
void select_node(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    Tn u_org = g.get_org_label(u);
    assert(vc.S[u_org] == std::nullopt);
    gs.label_count++;
    vc.S[u_org] = true;
    vc.cost += g.W(u);
    for (auto &&v : g[u])
        gs.push_search(v);
    g.remove_node(u);
}

template <typename Tn, typename Tw>
bool neighborhood_reduction(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    if (g.NW(u) <= g.W(u)) {
        vc.r1 += g.D(u) + 1;
        select_neighborhood(g, vc, gs, u);
        return true;
    }
    return false;
}

template <typename Tn, typename Tw>
bool twin_fold(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    assert(g.D(u) > 0);
    Tn first_neighbor = *(std::end(g[u]) - 1);
    bool found = false;
    for (auto &&v : g[first_neighbor]) {
        if (v != u && g.is_twin(u, v)) {
            vc.r2 += 1;
            g.fold_twin(u, v);
            found = true;
        }
    }
    if (found) {
        gs.push_search(u);
        for (auto &&v : g[u])
            gs.push_search(v);
        return true;
    }
    return false;
}

template <typename Tn, typename Tw>
bool domination_reduction(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    for (auto &&v : g[u]) {
        if (g.W(v) >= g.W(u) && g.is_dominating(u, v)) {
            vc.r3 += 1;
            select_node(g, vc, gs, u);
            return true;
        }
        if (g.W(v) <= g.W(u) && g.is_dominating(v, u)) {
            vc.r3 += 1;
            select_node(g, vc, gs, v);
            return true;
        }
    }
    return false;
}

template <typename Tn, typename Tw, typename It>
void neighborhood_difference(const reduction_graph<Tn, Tw> &g, Tn u, Tn v, It res, size_t cutoff) {
    auto &&f1 = std::begin(g[u]), l1 = std::end(g[u]);
    auto &&f2 = std::begin(g[v]), l2 = std::end(g[v]);
    size_t t = 0;
    while (f1 != l1 && f2 != l2) {
        if (*f1 < *f2) {
            if (*f1 != v) {
                *res = *f1;
                ++res;
                ++t;
                if (t > cutoff)
                    return;
            }
            ++f1;
        } else if (*f2 < *f1)
            ++f2;
        else {
            ++f1;
            ++f2;
        }
    }
    std::copy(f1, l1, res);
}

template <typename Tn, typename Tw>
bool neighbor_meta_reduction(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    std::vector<Tn> tmp;
    for (auto &&v : g[u]) {
        if (g.W(v) <= g.W(u) || (g.D(v) > g.D(u) && g.D(v) - g.D(u) > max_small_solve))
            continue;
        neighborhood_difference(g, v, u, std::back_inserter(tmp), max_small_solve);
        if (tmp.size() <= max_small_solve) {
            gs.sms.reset();
            for (auto &&_u : tmp) {
                gs.sms.add_node(_u, g.W(_u));
                for (auto &&_v : g[_u]) {
                    gs.sms.add_edge(_u, _v);
                }
            }

            Tw C = 0, VC = gs.sms.solve();
            for (auto &&w : tmp)
                C += g.W(w);
            if (C - VC + g.W(u) <= g.W(v)) {
                vc.r4 += 1;
                select_node(g, vc, gs, u);
                return true;
            }
        }
        tmp.clear();
    }
    return false;
}

template <typename Tn, typename Tw>
bool neighborhood_meta_reduction(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    if (g.D(u) > max_small_solve)
        return false;

    gs.sms.reset();
    for (auto &&v : g[u]) {
        gs.sms.add_node(v, g.W(v));
        for (auto &&w : g[v])
            gs.sms.add_edge(v, w);
    }

    if (g.W(u) >= g.NW(u) - gs.sms.solve()) {
        vc.r5 += g.D(u) + 1;
        select_neighborhood(g, vc, gs, u);
        return true;
    }
    return false;
}

template <typename Tn, typename Tw>
bool independent_fold(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    assert(g.W(u) < g.NW(u));
    Tn min_neighbor = *std::min_element(std::begin(g[u]), std::end(g[u]), [&](auto &&a, auto &&b) { return g.W(a) < g.W(b); });
    if (g.W(u) >= g.NW(u) - g.W(min_neighbor)) {
        if (g.has_independent_neighbors(u)) { // could be u and none of N(u)
            gs.label_count += g.D(u);
            vc.r6 += g.D(u);
            vc.cost += g.W(u);
            g.fold_neighborhood(u);
            gs.extend_to_new_node(g.size() - 1);
            vc.extend_to_new_node();
            for (auto &&v : g[g.size() - 1])
                gs.push_search(v);
        } else { // we need at least one neighbor, can always choose neighborhood
            vc.r6 += g.D(u) + 1;
            select_neighborhood(g, vc, gs, u);
        }
        return true;
    }
    return false;
}

template <typename Tn, typename Tw>
bool isolated_fold(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, Tn u) {
    if (g.is_isolated(u)) {
        vc.cost += g.W(u) * g.D(u); // we will exclude exactly 1 node from this clique
        g.fold_isolated(u);
        for (auto &&v : g[u]) {
            gs.push_search(v);
            for (auto &&w : g[v])
                gs.push_search(w);
        }
        vc.r7 += 1;
        gs.label_count++;
        return true;
    }
    return false;
}

template <typename Tn, typename Tw>
bool reduction_critial_weight(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs) {
    std::vector<std::tuple<Tn, Tn, Tw>> edges;
    Tn s = g.size() * 2, t = (g.size() * 2) + 1;
    for (Tn u = 0; u < g.size(); ++u) {
        if (!g.is_active(u))
            continue;
        edges.push_back({s, u, g.W(u)});
        edges.push_back({g.size() + u, t, g.W(u)});
        for (auto &&v : g[u]) {
            edges.push_back({u, g.size() + v, g.W(u)});
        }
    }
    flow_graph<Tn, Tw> fg(t + 1, edges);
    fg.solve(s, t);
    std::vector<bool> cs(g.size(), false);
    for (auto &&[u, w] : fg[s]) {
        cs[u] = w > 0;
    }
    for (Tn u = 0; u < g.size(); u++) {
        if (g.is_active(u) && cs[u]) {
            for (auto &&v : g[u])
                cs[v] = false;
        }
    }
    bool res = false;
    std::vector<Tn> rn;
    for (Tn u = 0; u < g.size(); u++) {
        if (g.is_active(u) && cs[u]) {
            rn.push_back(u);
            res = true;
        }
    }
    for (auto &&u : rn) {
        vc.r8 += g.D(u) + 1;
        select_neighborhood(g, vc, gs, u);
    }
    return res;
}

template <typename Tn, typename Tw>
void reduce_graph(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, bool do_critical = false) {
    bool critical = false;
    do {
        size_t rule = 0;
        while (rule < gs.num_local_reduction_rules) {
            if (gs.search[rule].empty()) {
                rule++;
            } else {
                Tn u = gs.pop_search(rule);
                if (u >= g.size() || !g.is_active(u) || g.D(u) > 20)
                    continue;
                bool found = false;
                switch ((reduction_rules)rule) {
                case reduction_rules::neighborhood_reduction:
                    found = neighborhood_reduction(g, vc, gs, u);
                    break;
                case reduction_rules::neighborhood_meta_reduction:
                    found = neighborhood_meta_reduction(g, vc, gs, u);
                    break;
                case reduction_rules::twin_fold:
                    found = twin_fold(g, vc, gs, u);
                    break;
                case reduction_rules::domination_reduction:
                    found = domination_reduction(g, vc, gs, u);
                    break;
                case reduction_rules::independent_fold:
                    found = independent_fold(g, vc, gs, u);
                    break;
                case reduction_rules::isolated_fold:
                    found = isolated_fold(g, vc, gs, u);
                    break;
                case reduction_rules::neighbor_meta_reduction:
                    found = neighbor_meta_reduction(g, vc, gs, u);
                    break;

                default:
                    break;
                }
                if (found)
                    rule = 0;
            }
        }
        if (do_critical)
            critical = reduction_critial_weight(g, vc, gs);
    } while (critical);
}
