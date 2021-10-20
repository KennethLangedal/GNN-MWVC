#pragma once
#include "gnn_inference.hpp"

namespace gnn {
    struct linear_layer_training {
        linear_layer l;

        mutable matrix in_copy, grad_out, grad_W, grad_bias, vel_W, vel_bias;

        linear_layer_training(size_t dim_in = 0, size_t dim_out = 0, size_t seed = 0);

        const matrix &forward(const matrix &in) const;
        const matrix &backward(const matrix &grad_in) const;
    };

    struct graph_layer_training {
        graph_layer l;

        mutable matrix grad_out;

        const matrix &forward(const matrix &in, const reduction_graph<Tn, Tw> &g) const;
        const matrix &backward(const matrix &grad_in, const reduction_graph<Tn, Tw> &g) const;
    };

    struct ReLU_training {
        ReLU l;

        mutable matrix in_copy, grad_out;

        const matrix &forward(const matrix &in) const;
        const matrix &backward(const matrix &grad_in) const;
    };

    struct sigmoid_training {
        sigmoid l;

        mutable matrix in_copy, grad_out;

        const matrix &forward(const matrix &in) const;
        const matrix &backward(const matrix &grad_in) const;
    };

    using component_training = std::variant<linear_layer_training, graph_layer_training, ReLU_training, sigmoid_training>;

    struct model_training {
        std::string name;
        std::vector<component_training> layers;

        model_training(std::string name = "");

        void add_layer(const component_training &c);

        const matrix &predict(const matrix &in, const reduction_graph<Tn, Tw> &g) const;
        const matrix &backprop(const matrix &grad_in, const reduction_graph<Tn, Tw> &g) const;

        friend std::ostream &operator<<(std::ostream &os, const model_training &m);
        friend std::istream &operator>>(std::istream &is, model_training &m);
    };

    std::ostream &operator<<(std::ostream &os, const model_training &m);
    std::istream &operator>>(std::istream &is, model_training &m);

    Tw MSE_loss(const matrix &x, const matrix &y);
    matrix MSE_grad(const matrix &x, const matrix &y);

    void SGD_step(model_training &m, float lr = 0.1, float momentum = 0.9, float weight_decay = 0);
    void zero_grad(model_training &m);
}