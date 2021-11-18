#include "mwvc_reductions.hpp"

template <typename Tn, typename Tw>
void medium_solve_req(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs) {

    std::vector<Tn> nodes(g.size());
    std::iota(std::begin(nodes), std::end(nodes), 0);
    std::sort(std::begin(nodes), std::end(nodes), [&](auto a, auto b) { return g.D(a) > g.D(b); });

    size_t k = std::max(g.size() / 4ull, 50ull), tk = 0;
    while (tk < nodes.size() && g.D(nodes[tk]) > tk)
        ++tk;

    if (tk >= k) { // degree-k

        vertex_cover<Tn, Tw> vc_copy = vc;
        size_t t = g.get_timestamp();
        // choose every node
        for (size_t i = 0; i < tk; ++i) {
            select_node(g, vc, gs, nodes[i]);
        }
        g.relable_graph();
        medium_solve_req(g, vc, gs);
        unfold_graph(g, vc, gs, t);
        vertex_cover<Tn, Tw> tmp_best = vc;
        vc = vc_copy;

        // try every neighborhood
        for (size_t i = 0; i < tk; ++i) {
            select_neighborhood(g, vc, gs, nodes[i]);
            g.relable_graph();
            medium_solve_req(g, vc, gs);
            unfold_graph(g, vc, gs, t);
            if (tmp_best.cost > vc.cost)
                tmp_best = vc;
            vc = vc_copy;
        }

        vc = tmp_best;

    } else { // branch
        size_t t1 = g.get_timestamp();

        reduce_graph(g, vc, gs, true);
        g.relable_graph();

        if (g.size() == 0) {
            unfold_graph(g, vc, gs, t1);
            return;
        }

        vertex_cover<Tn, Tw> vc_copy = vc;

        size_t t2 = g.get_timestamp();

        // pick node to branch
        Tn u = 0;
        for (Tn v = 1; v < g.size(); ++v) {
            if (g.D(v) > g.D(u))
                u = v;
        }

        select_neighborhood(g, vc, gs, u);
        g.relable_graph();
        medium_solve_req(g, vc, gs);
        unfold_graph(g, vc, gs, t2);

        vertex_cover<Tn, Tw> tmp_best = vc;
        vc = vc_copy;

        select_node(g, vc, gs, u);
        g.relable_graph();
        medium_solve_req(g, vc, gs);
        unfold_graph(g, vc, gs, t2);

        if (vc.cost > tmp_best.cost) {
            vc = tmp_best;
        }

        unfold_graph(g, vc, gs, t1);
    }
}

template <typename Tn, typename Tw>
void medium_solve(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs, std::vector<Tn> &nodes) {
    std::sort(std::begin(nodes), std::end(nodes));
    std::vector<Tw> weights(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        weights[i] = g.W(nodes[i]);
    }
    std::vector<std::pair<Tn, Tn>> edges;
    for (auto &&u : nodes) {
        Tn u_id = std::distance(std::begin(nodes), std::lower_bound(std::begin(nodes), std::end(nodes), u));
        for (auto &&v : g[u]) {
            if (v < u)
                continue;
            Tn v_id = std::distance(std::begin(nodes), std::lower_bound(std::begin(nodes), std::end(nodes), v));
            edges.push_back({u_id, v_id});
        }
    }
    reduction_graph<Tn, Tw> new_g(weights, edges);
    vertex_cover<Tn, Tw> new_vc(weights.size());
    graph_search<Tn> new_gs(weights.size());

    medium_solve_req(new_g, new_vc, new_gs);

    for (size_t i = 0; i < nodes.size(); ++i) {
        if (!g.is_active(nodes[i]))
            continue;
        if (!(*new_vc.S[i])) {
            select_neighborhood(g, vc, gs, nodes[i]);
        } else {
            select_node(g, vc, gs, nodes[i]);
        }
    }
}