#include "local_search.hpp"
#include "medium_solve.hpp"
#include "mwvc_reductions.hpp"
#include "small_solve.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>

using namespace std;

using Tn = uint32_t;
using Tw = uint32_t;

constexpr size_t CRITICAL_LIMIT = 1000;

struct test_graph {
    reduction_graph<Tn, Tw> g;
    vector<Tw> weights;
    vector<pair<Tn, Tn>> edges;
    string name;
    size_t N, E;
};

test_graph parse_graph(filesystem::path path) {
    string name = path.filename().stem();
    ifstream fs(path);
    if (!fs.is_open()) {
        cout << "Error opening graph file" << endl;
        return {reduction_graph<Tn, Tw>({}, {}), {}, {}, name, 0, 0};
    }
    size_t E, N;
    fs >> E >> N;
    vector<Tw> weights(N);
    for (auto &&w : weights)
        fs >> w;

    vector<pair<Tn, Tn>> edges(E);
    for (auto &&[u, v] : edges) {
        fs >> u >> v;
        if (u > N || v > N || u == v) {
            cout << name << "," << N << "," << E << ","
                 << "Invalid edge " << u << "," << v << endl;
            exit(0);
        }
        if (--u > --v)
            swap(u, v);
    }
    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));
    E = edges.size();

    return {reduction_graph<Tn, Tw>(weights, edges), weights, edges, name, N, E};
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
        }
    }
    return res;
}

size_t initial_reduction_cost = 0;
size_t after_initial_reductions = 0;

vertex_cover<Tn, Tw> gnn_solve(reduction_graph<Tn, Tw> &g, graph_search<Tn> &gs, bool verbose, int relable_interval = -1) {
    vertex_cover<Tn, Tw> res(g.size());
    vector<Tn> nodes(g.size());

    reduce_graph(g, res, gs, g.size() < CRITICAL_LIMIT);

    size_t i = g.size(), N = g.size(), j = 0;

    size_t t = 0;
    bool first_it = true;

    while (g.size() > 0) {
        if (i == N || (relable_interval > 0 && j > relable_interval) || (relable_interval < 0 && j > 0 && gs.label_count > (N / 10))) {
            small_solve_dfs(g, res, gs);

            g.relable_graph();

            if (first_it) {
                first_it = false;
                t = g.get_timestamp();
                initial_reduction_cost = res.cost;
                after_initial_reductions = g.size();
            }

            gs.label_count = 0;
            N = g.size();
            iota(begin(nodes), begin(nodes) + N, 0);

            sort(begin(nodes), begin(nodes) + N, [&](auto &&a, auto &&b) {
                return g.W(a) > g.W(b) || (g.W(a) == g.W(b) && g.D(a) < g.D(b));
            });

            i = 0;
            j = 0;
            if (verbose)
                cout << "Remaining nodes: " << g.size() << "          " << flush << (char)13;
        } else if (res.S[g.get_org_label(nodes[i])].has_value() && *res.S[g.get_org_label(nodes[i])]) {
            j++;
            i++;
        } else if (g.is_active(nodes[i])) {
            select_neighborhood(g, res, gs, nodes[i]);
            i++;
            reduce_graph(g, res, gs, g.size() < CRITICAL_LIMIT);
        } else {
            i++;
        }
    }

    unfold_graph(g, res, gs, t);
    return res;
}

int main(int narg, char **arg) {

    if (narg != 6) {
        cout << "Usage: ./QUICK_VC [graph] [result file] [time] [k (< 0 = auto)] [0 = silent, 1 = verbose]" << endl;
        return 0;
    }

    string graph_path(arg[1]), out_path(arg[2]);
    double t_max = stod(arg[3]);
    int k = stoi(arg[4]);
    bool verbose = arg[5][0] == '1';
    ofstream os(out_path);
    if (!os.is_open()) {
        cout << "Error opening result file" << endl;
        return 0;
    }

    auto t = parse_graph(graph_path);
    if (t.N == 0) {
        return 0;
    }

    if (verbose)
        cout << t.name << ", N = " << t.N << ", E = " << t.E << endl;

    reduction_graph<Tn, Tw> g_org = t.g;

    graph_search<uint32_t> gs(t.g.size());

    // vector<uint32_t> quality_at_time(100, 0);

    auto t1 = chrono::high_resolution_clock::now(), t2 = t1, t3 = t1;

    auto res = gnn_solve(t.g, gs, verbose, k);

    auto tgnn = chrono::high_resolution_clock::now();
    double time_gnn = chrono::duration<double>(tgnn - t1).count();
    size_t cost_gnn = res.cost;

    // size_t i = time_gnn / 10;

    if (verbose) {
        cout << "QUCK-VC done in " << chrono::duration<double>(tgnn - t1).count() << "s, cost: " << res.cost << endl;
    }

    if (t.g.size() == 0) {
        unfold_graph(t.g, res, gs, 0);
        // fill(begin(quality_at_time) + i, end(quality_at_time), res.cost);
        if (verbose)
            cout << "Vertex cover cost: " << res.cost << ", found in " << time_gnn << "s, " << time_gnn << " total time" << endl;
        else
            cout << t.name << "," << t.N << "," << t.E << "," << after_initial_reductions << "," << cost_gnn << "," << time_gnn << "," << res.cost << "," << time_gnn << endl;

        // cout << t.name << "," << t.N << "," << t.E << ",";
        // for (auto &&c : quality_at_time) {
        //     cout << c << ",";
        // }
        // cout << endl;

        for (Tn u = 0; u < t.N; ++u) {
            os << (*res.S[u] ? 1 : 0) << endl;
        }
        return 0;
    }

    local_search ls(res, t.g);

    t2 = chrono::high_resolution_clock::now();
    t3 = t2;

    size_t step_size = 1 << 16;
    size_t total = 0;
    while (time_gnn + chrono::duration<double>(chrono::high_resolution_clock::now() - t2).count() < t_max) {
        total += step_size;
        if (ls.search(step_size)) {
            t3 = chrono::high_resolution_clock::now();
            step_size = min(step_size * 2, 1ul << 16);
            if (verbose)
                cout << time_gnn + chrono::duration<double>(t3 - t2).count() << "," << ls.get_best_cost() + initial_reduction_cost << endl;
        } else {
            step_size = max(step_size / 2, 1ul << 10);
        }

        // if ((i + 1) * 10 < time_gnn + chrono::duration<double>(chrono::high_resolution_clock::now() - t2).count()) {
        //     quality_at_time[i++] = ls.get_best_cost() + initial_reduction_cost;
        // }
    }
    auto t4 = chrono::high_resolution_clock::now();
    ls.get_cover(res, t.g);

    unfold_graph(t.g, res, gs, 0);

    // quality_at_time.back() = res.cost;

    // cout << total << endl;

    if (!validate(g_org, res)) {
        cout << "Result is not a vertex cover" << endl;
        return 0;
    }

    if (verbose)
        cout << "Vertex cover cost: " << res.cost << ", found in " << chrono::duration<double>(t3 - t2).count() + time_gnn << "s, " << chrono::duration<double>(t4 - t2).count() + time_gnn << " total time" << endl;
    else
        cout << t.name << "," << t.N << "," << t.E << "," << after_initial_reductions << "," << cost_gnn << "," << time_gnn << "," << res.cost << "," << chrono::duration<double>(t3 - t2).count() + time_gnn << endl;

    // cout << t.name << "," << t.N << "," << t.E << ",";
    // for (auto &&c : quality_at_time) {
    //     cout << c << ",";
    // }
    // cout << endl;

    for (Tn u = 0; u < t.N; ++u) {
        os << (*res.S[u] ? 1 : 0) << endl;
    }

    return 0;
}