#include "gnn_training.hpp"
#include "mwvc_reductions.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <optional>

using namespace std;

using Tn = uint32_t;
using Tw = uint32_t;

struct test_graph {
    matrix x;
    reduction_graph<Tn, Tw> g;        // Original graph
    reduction_graph<Tn, float> mg;    // Scaled float weights graph
    optional<vertex_cover<Tn, Tw>> s; // Solution
    string name;
    size_t N, E;
};

test_graph parse_graph(filesystem::path path) {
    string name = path.filename().stem();
    ifstream fs(path);
    size_t E, N;
    fs >> E >> N;
    vector<float> scaled_weights(N);
    vector<Tw> weights(N);
    Tw w;
    for (Tn u = 0; u < N; ++u) {
        fs >> w;
        scaled_weights[u] = ((float)w) / 120.0f;
        weights[u] = w;
    }
    vector<pair<Tn, Tn>> edges(E);
    for (auto &&[u, v] : edges) {
        fs >> u >> v;
        if (--u > --v)
            swap(u, v);
    }
    sort(begin(edges), end(edges));

    matrix x(N, 1);
    for (Tn u = 0; u < N; ++u) {
        x(u, 0) = scaled_weights[u];
    }

    return {x, reduction_graph<Tn, Tw>(weights, edges), reduction_graph<Tn, float>(scaled_weights, edges), nullopt, name, N, E};
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

void solve(reduction_graph<Tn, Tw> &g, vertex_cover<Tn, Tw> &res, graph_search<Tn> &gs, const vector<Tn> &nodes) {
    reduce_graph(g, res, gs, false);
    for (auto &&u : nodes) {
        if (!g.is_active(u))
            continue;

        select_neighborhood(g, res, gs, u);
        reduce_graph(g, res, gs, false);
    }
    unfold_graph(g, res, gs, 0);
}

vertex_cover<Tn, Tw> quick_solve(reduction_graph<Tn, Tw> &g, bool full = false) {
    vertex_cover<Tn, Tw> res(g.size());
    vector<Tn> nodes(g.size());
    iota(begin(nodes), end(nodes), 0);
    sort(begin(nodes), end(nodes), [&](auto &&a, auto &&b) {
        return g.W(a) > g.W(b) || (g.W(a) == g.W(b) && g.D(a) < g.D(b));
    });
    if (full)
        num_local_reduction_rules = 7;
    graph_search<Tn> gs(g.size());
    reduce_graph(g, res, gs, false);
    if (full)
        num_local_reduction_rules = 6;

    solve(g, res, gs, nodes);
    return res;
}

vertex_cover<Tn, Tw> gnn_solve(matrix &x, reduction_graph<Tn, float> &mg, reduction_graph<Tn, Tw> &g, gnn::model &m) {
    vertex_cover<Tn, Tw> res(g.size());
    auto &&out = m.predict(x, mg);
    vector<Tn> nodes(g.size());
    iota(begin(nodes), end(nodes), 0);
    sort(begin(nodes), end(nodes), [&](auto &&a, auto &&b) {
        if (abs(out(a, 0) - out(b, 0)) > __FLT_EPSILON__)
            return out(a, 0) < out(b, 0);
        return g.W(a) > g.W(b) || (g.W(a) == g.W(b) && g.D(a) < g.D(b));
    });
    graph_search<Tn> gs(g.size());
    solve(g, res, gs, nodes);
    return res;
}

vertex_cover<Tn, Tw> gnn_solve_full(reduction_graph<Tn, Tw> &g, gnn::model &m) {
    num_local_reduction_rules = 7;
    vertex_cover<Tn, Tw> res(g.size());
    graph_search<Tn> gs(g.size());
    reduce_graph(g, res, gs, false);
    num_local_reduction_rules = 6;
    vector<Tn> new_label(g.size()), old_label;
    vector<float> weights;
    size_t t = 0;
    for (size_t i = 0; i < g.size(); ++i) {
        if (g.is_active(i)) {
            weights.push_back((float)g.W(i) / 120.0f);
            old_label.push_back(i);
            new_label[i] = t++;
        }
    }
    // cout << "Size after reduction: " << weights.size() << ", Cost: " << res.cost << endl;

    vector<pair<Tn, Tn>> edges;
    for (size_t u = 0; u < g.size(); ++u) {
        if (!g.is_active(u))
            continue;
        for (auto &&v : g[u]) {
            if (v < u)
                continue;
            edges.push_back({new_label[u], new_label[v]});
        }
    }
    matrix x(weights.size(), 1);
    for (Tn u = 0; u < weights.size(); ++u) {
        x(u, 0) = weights[u];
    }

    reduction_graph<Tn, float> mg(weights, edges);
    auto &&out = m.predict(x, mg);
    vector<Tn> nodes(mg.size());
    iota(begin(nodes), end(nodes), 0);
    sort(begin(nodes), end(nodes), [&](auto &&a, auto &&b) {
        if (abs(out(a, 0) - out(b, 0)) > __FLT_EPSILON__)
            return out(a, 0) < out(b, 0);
        return mg.W(a) > mg.W(b) || (mg.W(a) == mg.W(b) && mg.D(a) < mg.D(b));
    });
    transform(begin(nodes), end(nodes), begin(nodes), [&](auto &&u) { return old_label[u]; });
    solve(g, res, gs, nodes);
    return res;
}

int main(int narg, char **arg) {

    if (narg < 3 || narg > 4) {
        cout << "Usage: ./gnn_test [model] [graph] (optional [solution])" << endl;
        return 0;
    }
    gnn::model m;
    string model_path(arg[1]), graph_path(arg[2]);
    ifstream fs(model_path);
    fs >> m;

    auto t = parse_graph(graph_path);

    if (narg == 4) {
        string solution_path(arg[3]);
        fstream fs_solution(solution_path);
        t.s = vertex_cover<Tn, Tw>(t.N);
        int v;
        for (Tn u = 0; u < t.N; ++u) {
            fs_solution >> v;
            t.s->S[u] = v == 1;
            if (v == 1)
                t.s->cost += t.g.W(u);
        }
    }

    cout << t.name << ", N = " << t.N << ", E = " << t.E << endl;

    if (t.s.has_value() && !validate(t.g, *t.s))
        cout << "Exact solve not a vertex cover!" << endl;

    if (t.s.has_value())
        cout << "Exact = " << t.s->cost << endl;

    vector<Tw> cost_gnn, cost_quick;
    vector<double> time_gnn, time_quick;

    auto &&time_solve = [&](bool gnn, bool full, size_t R) {
        num_local_reduction_rules = R;
        vertex_cover<Tn, Tw> gs(0);

        auto t1 = chrono::high_resolution_clock::now();

        if (gnn)
            gs = full ? gnn_solve_full(t.g, m) : gnn_solve(t.x, t.mg, t.g, m);
        else
            gs = quick_solve(t.g, full);

        auto t2 = chrono::high_resolution_clock::now();

        chrono::duration<double> time = t2 - t1;

        if (!validate(t.g, gs))
            cout << "Not a vertex cover!" << endl;

        if (gnn) {
            cost_gnn.push_back(gs.cost);
            time_gnn.push_back(time.count());
        } else {
            cost_quick.push_back(gs.cost);
            time_quick.push_back(time.count());
        }
        cout << (gnn ? "GNN R" : "Quick R") << R << " = " << gs.cost;
        if (t.s.has_value())
            cout << " (" << ((float)(gs.cost - t.s->cost)) / t.s->cost * 100.0f << "%)";
        cout << " in " << time.count() << " s" << endl;
    };

    size_t nr = 7;
    for (int R = 0; R < nr; ++R) {
        time_solve(false, false, R);
        time_solve(true, false, R);
    }

    time_solve(false, true, 7);
    time_solve(true, true, 7);

    for (size_t i = 0; i < cost_gnn.size(); ++i) {
        cout << cost_quick[i] << "\t" << cost_gnn[i] << "\t";
    }
    cout << endl;
    for (size_t i = 0; i < time_gnn.size(); ++i) {
        cout << time_quick[i] << "\t" << time_gnn[i] << "\t";
    }
    cout << endl;

    return 0;
}