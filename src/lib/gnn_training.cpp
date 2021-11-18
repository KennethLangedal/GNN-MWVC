#include "gnn_training.hpp"
#include <cmath>
#include <numeric>

using namespace gnn;

linear_layer_training::linear_layer_training(size_t dim_in, size_t dim_out, size_t seed)
    : l(dim_in, dim_out, seed), grad_W(dim_in, dim_out), grad_bias(1, dim_out), vel_W(dim_in, dim_out), vel_bias(1, dim_out) {
}

void linear_layer_training::forward(const matrix &in, matrix &out) const {
    in_copy.resize(in.get_height(), in.get_width());
    std::copy(std::begin(in.raw()), std::end(in.raw()), std::begin(in_copy.raw()));
    l.forward(in, out);
}

void linear_layer_training::backward(const matrix &grad_in, matrix &grad_out) const {
    // Weights
    dot(in_copy, grad_in, grad_W, true, false, 1.0f);
    for (size_t i = 0; i < grad_in.get_height(); i++)
        std::transform(std::begin(grad_in[i]), std::end(grad_in[i]), std::begin(grad_bias.raw()), std::begin(grad_bias.raw()), std::plus());

    // Backwards flow
    dot(grad_in, l.W, grad_out, false, true, 0.0f);
}

void graph_layer_training::forward(const matrix &in, matrix &out, const reduction_graph<Tn, Tw> &g) const {
    l.forward(in, out, g);
}

void graph_layer_training::backward(const matrix &grad_in, matrix &grad_out, const reduction_graph<Tn, Tw> &g) const {
    grad_out.resize(grad_in.get_height(), (grad_in.get_width() - 3) / 2);
    std::fill(std::begin(grad_out.raw()), std::end(grad_out.raw()), 0.0f);

    for (gnn::Tn u = 0; u < g.size(); u++) {
        for (auto &&v : g[u]) {
            std::transform(std::begin(grad_out[u]), std::end(grad_out[u]), std::begin(grad_in[v]), std::begin(grad_out[u]), std::plus());
        }
        std::transform(std::begin(grad_out[u]), std::end(grad_out[u]), std::begin(grad_in[u]) + grad_out.get_width(), std::begin(grad_out[u]), std::plus());
    }
}

void ReLU_training::forward(const matrix &in, matrix &out) const {
    in_copy.resize(in.get_height(), in.get_width());
    std::copy(std::begin(in.raw()), std::end(in.raw()), std::begin(in_copy.raw()));
    l.forward(in, out);
}

void ReLU_training::backward(const matrix &grad_in, matrix &grad_out) const {
    grad_out.resize(grad_in.get_height(), grad_in.get_width());
    std::transform(std::begin(in_copy.raw()), std::end(in_copy.raw()), std::begin(grad_in.raw()), std::begin(grad_out.raw()), [](auto &&z, auto &&g) { return z >= 0.0f ? g : 0.0f; });
}

void sigmoid_training::forward(const matrix &in, matrix &out) const {
    in_copy.resize(in.get_height(), in.get_width());
    std::copy(std::begin(in.raw()), std::end(in.raw()), std::begin(in_copy.raw()));
    l.forward(in, out);
}

void sigmoid_training::backward(const matrix &grad_in, matrix &grad_out) const {
    grad_out.resize(grad_in.get_height(), grad_in.get_width());
    auto &&f = [](auto &&x) { return 1.0f / (1.0f + expf(-x)); };
    std::transform(std::begin(in_copy.raw()), std::end(in_copy.raw()), std::begin(grad_in.raw()), std::begin(grad_out.raw()), [&](auto &&z, auto &&g) { return f(z) * (1.0f - f(z)) * g; });
}

model_training::model_training(std::string name) : name(name) {
}
void model_training::add_layer(const component_training &c) {
    layers.push_back(c);
}

template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void model_training::predict(const matrix &in, matrix &out, const reduction_graph<Tn, Tw> &g) const {
    assert(!layers.empty());

    in_copy.resize(in.get_height(), in.get_width());
    std::copy(std::begin(in.raw()), std::end(in.raw()), std::begin(in_copy.raw()));

    for (auto &&l : layers) {
        std::visit(overloaded{
                       [&](const auto &l) { l.forward(in_copy, out); },
                       [&](const graph_layer_training &l) { l.forward(in_copy, out, g); }},
                   l);
        std::swap(in_copy, out);
    }
    std::swap(in_copy, out);
}

void model_training::backprop(const matrix &grad_in, matrix &grad_out, const reduction_graph<Tn, Tw> &g) const {
    assert(!layers.empty());

    in_copy.resize(grad_in.get_height(), grad_in.get_width());
    std::copy(std::begin(grad_in.raw()), std::end(grad_in.raw()), std::begin(in_copy.raw()));

    for (auto &&l : layers) {
        std::visit(overloaded{
                       [&](const auto &l) { l.backward(in_copy, grad_out); },
                       [&](const graph_layer_training &l) { l.backward(in_copy, grad_out, g); }},
                   l);
        std::swap(in_copy, grad_out);
    }
    std::swap(in_copy, grad_out);
}

std::ostream &gnn::operator<<(std::ostream &os, const model_training &m) {
    os << m.name << std::endl;
    os << m.layers.size() << " Layers" << std::endl;
    for (auto &&l : m.layers) {
        std::visit(overloaded{
                       [&](const linear_layer_training &c) {
                           os << "Linear_Layer" << std::endl;
                           os << "Weights: " << c.l.W << std::endl;
                           os << "Bias: " << c.l.bias << std::endl
                              << std::endl;
                       },
                       [&](const graph_layer_training &c) {
                           os << "Graph_Layer" << std::endl
                              << std::endl;
                       },
                       [&](const ReLU_training &c) {
                           os << "ReLU_Activation" << std::endl
                              << std::endl;
                       },
                       [&](const sigmoid_training &c) {
                           os << "Sigmoid_Activation" << std::endl
                              << std::endl;
                       }},
                   l);
    }
    return os;
}

std::istream &gnn::operator>>(std::istream &is, model_training &m) {
    size_t n;
    std::string tmp;
    is >> m.name >> n >> tmp;
    for (size_t i = 0; i < n; i++) {
        is >> tmp;
        if (tmp == "Linear_Layer") {
            linear_layer_training l;
            is >> tmp >> l.l.W >> tmp >> l.l.bias;
            m.layers.emplace_back(std::move(l));
        } else if (tmp == "Graph_Layer") {
            m.layers.emplace_back(graph_layer_training());
        } else if (tmp == "ReLU_Activation") {
            m.layers.emplace_back(ReLU_training());
        } else if (tmp == "Sigmoid_Activation") {
            m.layers.emplace_back(sigmoid_training());
        }
    }
    return is;
}

float gnn::MSE_loss(const matrix &x, const matrix &y) {
    float loss = 0.0f;
    for (size_t i = 0; i < x.get_height(); i++) {
        // Sum of square error
        float se = std::transform_reduce(std::begin(x[i]), std::end(x[i]), std::begin(y[i]), 0.0f, std::plus<float>(), [&](auto &&v1, auto &&v2) { return (v1 - v2) * (v1 - v2); });
        loss += se / x.get_width();
    }
    return loss / x.get_height();
}

void gnn::MSE_grad(const matrix &x, const matrix &y, matrix &grad_out) {
    grad_out.resize(x.get_height(), x.get_width());
    for (size_t i = 0; i < x.get_height(); i++) {
        std::transform(std::begin(x[i]), std::end(x[i]), std::begin(y[i]), std::begin(grad_out[i]), [&](auto &&v1, auto &&v2) { return ((2.0f * (v1 - v2)) / x.get_width()); }); // / x.get_height()
    }
}

void gnn::SGD_step(model_training &m, size_t batch_size, float lr, float momentum, float weight_decay) {
    // L2 regularization
    if (weight_decay > 0.0f) {
        auto &&l2_grad = [&](auto &&grad, auto &&w) { return grad + (2.0f * weight_decay * w); };

        for (auto &&layer : m.layers) {
            std::visit(overloaded{
                           [&](auto &&l) {},
                           [&](linear_layer_training &l) {
                               std::transform(std::begin(l.grad_W.raw()), std::end(l.grad_W.raw()), std::begin(l.l.W.raw()), std::begin(l.grad_W.raw()), l2_grad);
                               std::transform(std::begin(l.grad_bias.raw()), std::end(l.grad_bias.raw()), std::begin(l.l.bias.raw()), std::begin(l.grad_bias.raw()), l2_grad);
                           }},
                       layer);
        }
    }
    // Weight update
    for (auto &&layer : m.layers) {
        std::visit(overloaded{
                       [&](auto &&l) {},
                       [&](linear_layer_training &l) {
                           // Update velocity
                           std::transform(std::begin(l.vel_W.raw()), std::end(l.vel_W.raw()), std::begin(l.grad_W.raw()), std::begin(l.vel_W.raw()), [&](auto &&vel, auto &&grad) { return (momentum * vel) + (grad / batch_size); });
                           std::transform(std::begin(l.vel_bias.raw()), std::end(l.vel_bias.raw()), std::begin(l.grad_bias.raw()), std::begin(l.vel_bias.raw()), [&](auto &&vel, auto &&grad) { return (momentum * vel) + (grad / batch_size); });

                           // Update weights
                           std::transform(std::begin(l.l.W.raw()), std::end(l.l.W.raw()), std::begin(l.vel_W.raw()), std::begin(l.l.W.raw()), [&](auto &&param, auto &&vel) { return param - (lr * vel); });
                           std::transform(std::begin(l.l.bias.raw()), std::end(l.l.bias.raw()), std::begin(l.vel_bias.raw()), std::begin(l.l.bias.raw()), [&](auto &&param, auto &&vel) { return param - (lr * vel); });
                       }},
                   layer);
    }
}

void gnn::zero_grad(model_training &m) {
    for (auto &&layer : m.layers) {
        std::visit(overloaded{
                       [&](auto &&l) {},
                       [&](linear_layer_training &l) {
                           std::fill(std::begin(l.grad_W.raw()), std::end(l.grad_W.raw()), 0.0f);
                           std::fill(std::begin(l.grad_bias.raw()), std::end(l.grad_bias.raw()), 0.0f);
                       }},
                   layer);
    }
}