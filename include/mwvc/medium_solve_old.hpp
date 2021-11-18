// #pragma once
// #include "small_solve.hpp"
// #include <functional>
// #include <stack>

// enum class dense_actions {
//     neighborhood_reduction,
//     node_reduction
// };

// class dense_mwvc_graph {
// private:
//     std::vector<std::vector<uint64_t>> adjacency_matrix;
//     std::vector<uint32_t> weights, neighborhood_weights, degree;
//     std::vector<bool> active, vc;
//     std::stack<std::pair<dense_actions, size_t>> log;
//     size_t N, cost;

// public:
//     dense_mwvc_graph(size_t N, const std::vector<uint32_t> &weights)
//         : adjacency_matrix(N, std::vector<uint64_t>((N + 63) / 64, 0ull)), weights(weights), neighborhood_weights(N, 0), degree(N, 0), active(N, true), vc(N, false), log{}, N(N), cost(0) {}

//     void add_edge(size_t u, size_t v) {
//         assert(u < N && v < N && (adjacency_matrix[u][v / 64] & (1ull << (v % 64ull))) == 0 && (adjacency_matrix[v][u / 64] & (1ull << (u % 64ull))) == 0);
//         adjacency_matrix[u][v / 64] |= 1ull << (v % 64ull);
//         adjacency_matrix[v][u / 64] |= 1ull << (u % 64ull);
//         neighborhood_weights[u] += weights[v];
//         neighborhood_weights[v] += weights[u];
//         degree[u]++;
//         degree[v]++;
//     }

//     void visit(size_t u, std::function<void(size_t)> f) {
//         assert(u < N);
//         uint64_t block;
//         for (size_t i = 0; i < adjacency_matrix[u].size(); ++i) {
//             block = adjacency_matrix[u][i];
//             while (block != 0) {
//                 f(i * 64ull + __builtin_ctzl(block));
//                 block ^= block & -block;
//             }
//         }
//     }

//     size_t size() const {
//         return N;
//     }

//     bool is_active(size_t u) const {
//         assert(u < N);
//         return active[u];
//     }

//     size_t get_log_size() const {
//         return log.size();
//     }

//     uint32_t get_cost() const {
//         return cost;
//     }

//     size_t get_degree(size_t u) const {
//         assert(u < N);
//         return degree[u];
//     }

//     void get_vc(std::vector<bool> &dest) const {
//         dest.resize(N);
//         std::copy(std::begin(vc), std::end(vc), std::begin(dest));
//     }

//     void set_vc(const std::vector<bool> &source) {
//         assert(source.size() == N);
//         std::copy(std::begin(source), std::end(source), std::begin(vc));
//         cost = 0;
//         for (size_t u = 0; u < N; ++u) {
//             if (vc[u])
//                 cost += weights[u];
//         }
//     }

//     // Select Neighborhood

//     void select_and_reduce_neighborhood(size_t u) {
//         assert(u < N && active[u]);
//         log.push({dense_actions::neighborhood_reduction, u});
//         active[u] = false;
//         visit(u, [&](size_t v) { active[v] = false; vc[v] = true; cost += weights[v]; });
//         visit(u, [&](size_t v) {
//             visit(v, [&](size_t w) {
//                 if (active[w]) {
//                     degree[w]--;
//                     neighborhood_weights[w] -= weights[v];
//                     adjacency_matrix[w][v / 64] ^= 1ull << (v % 64ull);
//                 }
//             });
//         });
//     }

//     void restore_neighborhood(size_t u) {
//         assert(u < N && !active[u]);
//         visit(u, [&](size_t v) {
//             visit(v, [&](size_t w) {
//                 if (active[w]) {
//                     degree[w]++;
//                     neighborhood_weights[w] += weights[v];
//                     adjacency_matrix[w][v / 64] |= 1ull << (v % 64ull);
//                 }
//             });
//         });
//         active[u] = true;
//         visit(u, [&](size_t v) { active[v] = true; });
//     }

//     // Select Node

//     void select_and_reduce_node(size_t u) {
//         assert(u < N && active[u]);
//         log.push({dense_actions::node_reduction, u});
//         active[u] = false;
//         vc[u] = true;
//         cost += weights[u];
//         visit(u, [&](size_t v) {
//             degree[v]--;
//             neighborhood_weights[v] -= weights[u];
//             adjacency_matrix[v][u / 64] ^= 1ull << (u % 64ull);
//         });
//     }

//     void restore_node(size_t u) {
//         assert(u < N && !active[u]);
//         active[u] = true;
//         visit(u, [&](size_t v) {
//             degree[v]++;
//             neighborhood_weights[v] += weights[u];
//             adjacency_matrix[v][u / 64] |= 1ull << (u % 64ull);
//         });
//     }

//     // Restore

//     void restore_graph(size_t to_time) {
//         while (log.size() > to_time) {
//             auto [action, node] = log.top();
//             log.pop();
//             switch (action) {
//             case dense_actions::neighborhood_reduction:
//                 restore_neighborhood(node);
//                 break;
//             case dense_actions::node_reduction:
//                 restore_node(node);
//                 break;

//             default:
//                 break;
//             }
//         }
//     }

//     // Neighborhood Rule

//     bool try_neighborhood_reduction_rule() {
//         bool res = false;
//         for (size_t u = 0; u < N; ++u) {
//             if (active[u] && neighborhood_weights[u] <= weights[u]) {
//                 select_and_reduce_neighborhood(u);
//                 res = true;
//             }
//         }
//         return res;
//     }
// };

// void reduce_dense_graph(dense_mwvc_graph &g) {
//     bool found = true;
//     while (found) {
//         found = g.try_neighborhood_reduction_rule();
//     }
// }

// void medium_solve(dense_mwvc_graph &g, std::vector<bool> &best, uint32_t &best_cost) {

//     std::vector<bool> tmp1, tmp2;

//     g.get_vc(tmp1);

//     size_t t1 = g.get_log_size();

//     reduce_dense_graph(g);

//     g.get_vc(tmp2);

//     size_t t2 = g.get_log_size();

//     std::vector<std::pair<uint32_t, size_t>> degrees;
//     for (size_t u = 0; u < g.size(); ++u) {
//         if (g.is_active(u))
//             degrees.push_back({g.get_degree(u), u});
//     }

//     if (degrees.size() == 0) {
//         if (best_cost > g.get_cost()) {
//             best_cost = g.get_cost();
//             g.get_vc(best);
//         }
//         return;
//     }

//     std::sort(std::begin(degrees), std::end(degrees));
//     size_t k = 0;
//     while (k < degrees.size() && degrees[k].first >= k)
//         ++k;

//     // if degree-k
//     if (k > degrees.size() / 3 && false) {

//     } else { // else branch
//         size_t branch_node = g.size();
//         for (size_t u = 0; u < g.size(); ++u) {
//             if (g.is_active(u) && (branch_node == g.size() || g.get_degree(branch_node) < g.get_degree(u)))
//                 branch_node = u;
//         }

//         g.select_and_reduce_neighborhood(branch_node);
//         medium_solve(g, best, best_cost);
//         g.restore_graph(t2);

//         g.set_vc(tmp2);

//         g.select_and_reduce_node(branch_node);
//         medium_solve(g, best, best_cost);

//         g.restore_graph(t1);

//         g.set_vc(tmp1)
//     }
// }