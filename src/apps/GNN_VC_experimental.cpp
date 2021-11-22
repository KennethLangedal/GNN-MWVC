#include "gnn_training.hpp"
#include "medium_solve.hpp"
#include "mwvc_reductions.hpp"
#include "small_solve.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <optional>

using namespace std;

using Tn = uint32_t;
using Tw = uint32_t;

constexpr size_t CRITICAL_LIMIT = 5000;

struct test_graph {
    reduction_graph<Tn, Tw> g;
    string name;
    size_t N, E;
};

test_graph parse_graph(filesystem::path path) {
    string name = path.filename().stem();
    ifstream fs(path);
    size_t E, N;
    fs >> E >> N;
    vector<Tw> weights(N);
    for (auto &&w : weights)
        fs >> w;

    vector<pair<Tn, Tn>> edges(E);
    for (auto &&[u, v] : edges) {
        fs >> u >> v;
        if (--u > --v)
            swap(u, v);
    }
    sort(begin(edges), end(edges));

    return {reduction_graph<Tn, Tw>(weights, edges), name, N, E};
}

bool validate(const reduction_graph<Tn, Tw> &g, const vertex_cover<Tn, Tw> &s) {
    for (Tn u = 0; u < g.size(); ++u) {
        for (auto &&v : g[u]) {
            if (!s.S[u].has_value() || !s.S[v].has_value() || (!*s.S[u] && !*s.S[v]))
                return false;
        }
    }
    Tw cost = 0;
    for (Tn u = 0; u < g.size(); u++) {
        if (*s.S[u])
            cost += g.W(u);
    }
    return cost == s.cost;
}

size_t r9 = 0;

size_t small_solve_dfs(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &vc, graph_search<Tn> &gs) {
    small_mwvc_solver sms;
    vector<Tn> component;
    vector<bool> visited(g.size(), false);
    stack<Tn> dfs_stack;
    size_t res = 0;

    for (Tn u = 0; u < g.size(); ++u) {
        if (visited[u] || !g.is_active(u))
            continue;
        component.clear();
        dfs_stack.push(u);
        visited[u] = true;

        while (!dfs_stack.empty()) {
            Tn v = dfs_stack.top();
            dfs_stack.pop();
            component.push_back(v);
            for (auto &&w : g[v]) {
                if (!visited[w]) {
                    visited[w] = true;
                    dfs_stack.push(w);
                }
            }
        }
        res++;
        if (component.size() < 75) {

            medium_solve(g, vc, gs, component);

            r9 += component.size();
        }
    }
    return res;
}

size_t labels_from_model = 0, mistakes_from_model = 0;

float WEIGHT_SCALE = 120.0f;

vertex_cover<Tn, Tw> gnn_solve(reduction_graph<Tn, Tw> &g, gnn::model &m, size_t relable_interval, bool GNN = true, bool reductions = true, bool small_solve = true) {
    r9 = 0;
    labels_from_model = 0;
    mistakes_from_model = 0;
    vertex_cover<Tn, Tw> res(g.size());
    vector<Tn> nodes(g.size());
    graph_search<Tn> gs(g.size());

    if (reductions)
        reduce_graph(g, res, gs, g.size() < CRITICAL_LIMIT);

    size_t i = g.size(), N = g.size(), j = 0;
    matrix x, out;

    while (g.size() > 0) {
        if (i == N || j > relable_interval) {
            if (small_solve)
                small_solve_dfs(g, res, gs);

            g.relable_graph();
            N = g.size();
            iota(begin(nodes), begin(nodes) + N, 0);

            if (GNN) {
                x.resize(N, 1);
                for (Tn u = 0; u < N; ++u)
                    x(u, 0) = (float)g.W(u) / WEIGHT_SCALE;
                m.predict(x, out, g);

                if (reductions) {
                    sort(begin(nodes), begin(nodes) + N, [&](auto &&a, auto &&b) {
                        return min(out(a, 0), 1.0f - out(a, 0)) < min(out(b, 0), 1.0f - out(b, 0));
                    });
                } else {
                    sort(begin(nodes), begin(nodes) + N, [&](auto &&a, auto &&b) {
                        return out(a, 0) < out(b, 0);
                    });
                }
            } else {
                sort(begin(nodes), begin(nodes) + N, [&](auto &&a, auto &&b) {
                    return g.W(a) > g.W(b) || (g.W(a) == g.W(b) && g.D(a) < g.D(b));
                });
            }

            i = 0;
            cout << "Testing " << (GNN ? "GNN" : "QUICK") << (reductions ? " with reductions" : "") << (small_solve ? " using small_solve" : "") << ", remaining nodes: " << g.size() << "                    " << flush << (char)13;
            j = 0;
        } else if (res.S[g.get_org_label(nodes[i])].has_value() && ((GNN && *res.S[g.get_org_label(nodes[i])] != (out(nodes[i], 0) > 0.5f)) || (!GNN && *res.S[g.get_org_label(nodes[i])]))) {
            mistakes_from_model++;
            j++;
            i++;
        } else if (g.is_active(nodes[i])) {
            if (GNN && reductions) {
                if (out(nodes[i], 0) > 0.5f) {
                    select_node(g, res, gs, nodes[i]);
                    labels_from_model++;
                } else {
                    labels_from_model += g.D(nodes[i]) + 1;
                    select_neighborhood(g, res, gs, nodes[i]);
                }
            } else {
                labels_from_model += g.D(nodes[i]) + 1;
                select_neighborhood(g, res, gs, nodes[i]);
            }
            i++;
            if (reductions)
                reduce_graph(g, res, gs, g.size() < CRITICAL_LIMIT);
        } else {
            i++;
        }
    }

    unfold_graph(g, res, gs, 0);
    return res;
}

int main(int narg, char **arg) {

    if (narg != 4) {
        cout << "Usage: ./gnn_test [model] [graph] [output]" << endl;
        cout << "Output format (GNN = G, QUICK = Q, reductions = R, small_solve = S, local_search = L, time = T, cost = C):" << endl;
        cout << "Name,N,E,GRSL,T,GRS,T,GRL,T,GR,T,GSL,T,GS,T,GL,T,G,T,QRSL,T,QRS,T,QRL,T,QR,T,QSL,T,QS,T,QL,T,Q,T," << endl;
        cout << "Last 10 values are Neighborhood_reduction,Twin_fold,Domination_reduction,Neighbor_meta,Neighborhood_meta,Independent_fold,Isolated_fold,Critical,Small_solve,GNN for GRSL" << endl;
        return 0;
    }

    gnn::model m;
    string model_path(arg[1]), graph_path(arg[2]), out_path(arg[3]);
    ifstream fs(model_path);
    ofstream os(out_path, ios_base::app);
    if (!fs.is_open() || !os.is_open()) {
        cout << "Error opening files" << endl;
        return 0;
    }

    fs >> m;

    auto t = parse_graph(graph_path);

    cout << t.name << ", N = " << t.N << ", E = " << t.E << endl;
    os << t.name << "," << t.N << "," << t.E << "," << std::flush;

    reduction_graph<Tn, Tw> g_org = t.g;

    size_t relable_interval = 10;

    auto run_test = [&](bool GNN, bool reductions, bool small_solve) {
        auto t1 = chrono::high_resolution_clock::now();
        auto res = gnn_solve(t.g, m, (reductions ? relable_interval : 100000), GNN, reductions, small_solve);
        if (!validate(g_org, res))
            cout << "Result not a vertex cover" << endl;
        auto t2 = chrono::high_resolution_clock::now();
        auto before_local = res;

        vector<uint32_t> deactive_weights(t.N, 0);
        vector<bool> tmp_active(t.N, false);

        for (uint32_t u = 0; u < t.N; ++u) {
            if (!(*res.S[u])) {
                for (auto &&v : t.g[u])
                    deactive_weights[v] += t.g.W(u);
            }
        }

        auto remove_node_from_vc = [&](uint32_t u) {
            res.S[u] = false;
            res.cost -= t.g.W(u);
            for (auto &&v : t.g[u]) {
                deactive_weights[v] += t.g.W(u);
                if (!(*res.S[v])) {
                    res.S[v] = true;
                    res.cost += t.g.W(v);
                    for (auto &&w : t.g[v]) {
                        deactive_weights[w] -= t.g.W(v);
                    }
                }
            }
        };

        bool improvement = true;
        while (improvement) {
            improvement = false;
            for (size_t u = 0; u < t.N; ++u) {
                if (!(*res.S[u])) {
                    uint32_t cost_improvement = 0;
                    for (auto &&v : t.g[u])
                        tmp_active[v] = true;
                    for (auto &&v : t.g[u]) {
                        if (!tmp_active[v])
                            continue;
                        if (deactive_weights[v] - t.g.W(u) < t.g.W(v)) {
                            cost_improvement += t.g.W(v) - (deactive_weights[v] - t.g.W(u));
                            for (auto &&w : t.g[v])
                                tmp_active[w] = false;
                        } else {
                            tmp_active[v] = false;
                        }
                    }
                    if (cost_improvement > t.g.W(u)) {
                        improvement = true;
                        for (auto &&v : t.g[u]) {
                            if (tmp_active[v]) {
                                remove_node_from_vc(v);
                            }
                        }
                    }
                    for (auto &&v : t.g[u])
                        tmp_active[v] = false;
                } else {
                    if (deactive_weights[u] < t.g.W(u)) {
                        improvement = true;
                        remove_node_from_vc(u);
                    }
                }
            }
        }

        if (!validate(g_org, res))
            cout << "Result not a vertex cover" << endl;
        auto t3 = chrono::high_resolution_clock::now();
        os << res.cost << "," << chrono::duration<double>(t3 - t1).count() << "," << before_local.cost << "," << chrono::duration<double>(t2 - t1).count() << "," << std::flush;
        return res;
    };

    auto res = run_test(true, true, true);
    size_t ss_count = r9, r10 = labels_from_model;
    run_test(true, true, false);
    run_test(true, false, true);
    run_test(true, false, false);
    run_test(false, true, true);
    run_test(false, true, false);
    run_test(false, false, true);
    run_test(false, false, false);

    os << res.r1 << "," << res.r2 << "," << res.r3 << "," << res.r4 << "," << res.r5 << "," << res.r6 << "," << res.r7 << "," << res.r8 << "," << ss_count << "," << r10 << endl;

    return 0;
}