#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <immintrin.h>

class small_mwvc_solver {
private:
    std::array<size_t, 16> labels = {};
    std::array<int32_t, 16> weights = {};
    std::array<uint16_t, 16> g = {};
    size_t N = 0;
    mutable int32_t cost = std::numeric_limits<int32_t>::max();
    mutable uint16_t s = 0;

public:
    void reset() {
        labels.fill(0ull);
        weights.fill(0ull);
        g.fill(0);
        N = 0;
        cost = std::numeric_limits<int32_t>::max();
        s = 0;
    }

    void add_node(size_t u, int32_t w) {
        assert(N < 16);
        labels[N] = u;
        weights[N] = w;
        ++N;
    }

    void add_edge(size_t u, size_t v) {
        auto u_pos = std::find(std::begin(labels), std::begin(labels) + N, u);
        auto v_pos = std::find(std::begin(labels), std::begin(labels) + N, v);
        if (u_pos == std::begin(labels) + N || v_pos == std::begin(labels) + N)
            return;
        u = std::distance(std::begin(labels), u_pos);
        v = std::distance(std::begin(labels), v_pos);
        g[u] |= 1 << v;
        g[v] |= 1 << u;
    }

    uint64_t solve() const {
        __m128i v = _mm_set_epi32(0, 1, 2, 3), best = _mm_set1_epi32(std::numeric_limits<int32_t>::max()), solutions = _mm_set1_epi32(0);
        __m128i costs, valid, node, edges, chosen_node, chosen_neighborhood, better;

        for (size_t i = 0; i < ((1 << N) + 3) / 4; ++i) {
            valid = _mm_set1_epi32(0xFFFFFFFF);
            costs = _mm_set1_epi32(0);
            for (size_t j = 0; j < N; ++j) {
                node = _mm_set1_epi32(1 << j);
                edges = _mm_set1_epi32(g[j]);
                chosen_node = _mm_cmpgt_epi32(_mm_and_si128(v, node), _mm_set1_epi32(0));
                chosen_neighborhood = _mm_cmpeq_epi32(_mm_and_si128(v, edges), edges);
                valid = _mm_and_si128(valid, _mm_or_si128(chosen_node, chosen_neighborhood));
                costs = _mm_add_epi32(costs, _mm_and_si128(chosen_node, _mm_set1_epi32(weights[j])));
            }
            costs = _mm_or_si128(_mm_andnot_si128(valid, _mm_set1_epi32(std::numeric_limits<int32_t>::max())), costs);
            better = _mm_cmplt_epi32(costs, best);
            best = _mm_or_si128(_mm_and_si128(better, costs), _mm_andnot_si128(better, best));
            solutions = _mm_or_si128(_mm_and_si128(better, v), _mm_andnot_si128(better, solutions));
            v = _mm_add_epi32(v, _mm_set1_epi32(4));
        }
        int32_t tmp_cost[4], tmp_s[4];
        _mm_store_si128((__m128i *)tmp_cost, best);
        _mm_store_si128((__m128i *)tmp_s, solutions);

        for (size_t i = 0; i < 4; ++i) {
            if (tmp_cost[i] < cost) {
                cost = tmp_cost[i];
                s = tmp_s[i];
            }
        }
        return cost;
    }

    bool in_s(size_t u) const {
        auto u_pos = std::find(std::begin(labels), std::end(labels), u);
        return (s & (1 << std::distance(std::begin(labels), u_pos))) > 0;
    }
};