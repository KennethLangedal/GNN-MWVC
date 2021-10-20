#include "gnn_inference.hpp"
#include <cmath>
#include <random>

using namespace gnn;

linear_layer::linear_layer(size_t dim_in, size_t dim_out, size_t seed)
    : W(dim_in, dim_out), bias(1, dim_out) {
    Tw lim = 1.0 / sqrt(dim_in + 1);
    std::uniform_real_distribution<Tw> dist(-lim, lim);
    std::mt19937 gen(seed);
    for (auto &&w : W.raw()) {
        w = dist(gen);
    }
    for (auto &&w : bias.raw()) {
        w = dist(gen);
    }
}

const matrix &linear_layer::forward(const matrix &in) const {
    dot(in, W, out, false, false, 0.0f);
    for (size_t i = 0; i < out.get_height(); i++) {
        std::transform(std::begin(out[i]), std::end(out[i]), std::begin(bias.raw()), std::begin(out[i]), std::plus());
    }
    return out;
}

const matrix &graph_layer::forward(const matrix &in, const reduction_graph<Tn, Tw> &g) const {
    out.resize(in.get_height(), (in.get_width() * 2) + 3);
    std::fill(std::begin(out.raw()), std::end(out.raw()), 0.0);

    for (Tn u = 0; u < g.size(); u++) {
        for (auto &&v : g[u]) {
            std::transform(std::begin(in[v]), std::end(in[v]), std::begin(out[u]), std::begin(out[u]), std::plus());
        }
        std::copy(std::begin(in[u]), std::end(in[u]), std::begin(out[u]) + in.get_width());
        *(std::begin(out[u]) + in.get_width() + 1) = (Tw)g.D(u) / (Tw)g.size();
        *(std::begin(out[u]) + in.get_width() + 2) = g.W(u);
        *(std::begin(out[u]) + in.get_width() + 3) = g.NW(u);
    }
    return out;
}

const matrix &ReLU::forward(const matrix &in) const {
    out.resize(in.get_height(), in.get_width());
    std::transform(std::begin(in.raw()), std::end(in.raw()), std::begin(out.raw()), [](auto &&x) { return std::max(x, 0.0f); });
    return out;
}

const matrix &sigmoid::forward(const matrix &in) const {
    out.resize(in.get_height(), in.get_width());
    std::transform(std::begin(in.raw()), std::end(in.raw()), std::begin(out.raw()), [](auto &&x) { return 1.0f / (1.0f + expf(-x)); });
    return out;
}

model::model(std::string name) : name(name) {
}

void model::add_layer(const component &c) {
    layers.push_back(c);
}

template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

const matrix &model::predict(const matrix &in, const reduction_graph<Tn, Tw> &g) const {
    assert(!layers.empty());

    std::visit(overloaded{
                   [&](auto &&l) { l.forward(in); },
                   [&](const graph_layer &l) { l.forward(in, g); }},
               layers.front());

    for (size_t i = 1; i < layers.size(); i++)
        std::visit(overloaded{
                       [&](auto &&l, auto &&prev) { l.forward(prev.out); },
                       [&](const graph_layer &l, auto &&prev) { l.forward(prev.out, g); }},
                   layers[i], layers[i - 1]);

    return std::visit([](auto &&l) -> const matrix & { return l.out; }, layers.back());
}

std::ostream &gnn::operator<<(std::ostream &os, const model &m) {
    os << m.name << std::endl;
    os << m.layers.size() << " Layers" << std::endl;
    for (auto &&l : m.layers) {
        std::visit(overloaded{
                       [&](const linear_layer &c) {
                           os << "Linear_Layer" << std::endl;
                           os << "Weights: " << c.W << std::endl;
                           os << "Bias: " << c.bias << std::endl
                              << std::endl;
                       },
                       [&](const graph_layer &c) {
                           os << "Graph_Layer" << std::endl
                              << std::endl;
                       },
                       [&](const ReLU &c) {
                           os << "ReLU_Activation" << std::endl
                              << std::endl;
                       },
                       [&](const sigmoid &c) {
                           os << "Sigmoid_Activation" << std::endl
                              << std::endl;
                       }},
                   l);
    }
    return os;
}

std::istream &gnn::operator>>(std::istream &is, model &m) {
    size_t n;
    std::string tmp;
    is >> m.name >> n >> tmp;
    for (size_t i = 0; i < n; i++) {
        is >> tmp;
        if (tmp == "Linear_Layer") {
            linear_layer l;
            is >> tmp >> l.W >> tmp >> l.bias;
            m.layers.emplace_back(std::move(l));
        } else if (tmp == "Graph_Layer") {
            m.layers.emplace_back(graph_layer());
        } else if (tmp == "ReLU_Activation") {
            m.layers.emplace_back(ReLU());
        } else if (tmp == "Sigmoid_Activation") {
            m.layers.emplace_back(sigmoid());
        }
    }
    return is;
}