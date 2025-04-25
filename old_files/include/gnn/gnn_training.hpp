#pragma once
#include "gnn_inference.hpp"

namespace gnn {
    struct linear_layer_training {
        linear_layer l;

        mutable matrix in_copy, grad_W, grad_bias, vel_W, vel_bias;

        linear_layer_training(size_t dim_in = 0, size_t dim_out = 0, size_t seed = 0);

        void forward(const matrix &in, matrix &out) const;
        void backward(const matrix &grad_in, matrix &grad_out) const;
    };

    struct graph_layer_training {
        graph_layer l;

        graph_layer_training(float WEIGHT_SCALE = 1200.0f) { l.WEIGHT_SCALE = WEIGHT_SCALE; }

        void forward(const matrix &in, matrix &out, const reduction_graph<Tn, Tw> &g) const;
        void backward(const matrix &grad_in, matrix &grad_out, const reduction_graph<Tn, Tw> &g) const;
    };

    struct ReLU_training {
        ReLU l;

        mutable matrix in_copy;

        void forward(const matrix &in, matrix &out) const;
        void backward(const matrix &grad_in, matrix &grad_out) const;
    };

    struct sigmoid_training {
        sigmoid l;

        mutable matrix in_copy;

        void forward(const matrix &in, matrix &out) const;
        void backward(const matrix &grad_in, matrix &grad_out) const;
    };

    using component_training = std::variant<linear_layer_training, graph_layer_training, ReLU_training, sigmoid_training>;

    struct model_training {
        std::string name;
        std::vector<component_training> layers;
        mutable matrix in_copy;

        model_training(std::string name = "");

        void add_layer(const component_training &c);

        void predict(const matrix &in, matrix &out, const reduction_graph<Tn, Tw> &g) const;
        void backprop(const matrix &grad_in, matrix &grad_out, const reduction_graph<Tn, Tw> &g) const;

        friend std::ostream &operator<<(std::ostream &os, const model_training &m);
        friend std::istream &operator>>(std::istream &is, model_training &m);
    };

    std::ostream &operator<<(std::ostream &os, const model_training &m);
    std::istream &operator>>(std::istream &is, model_training &m);

    float MSE_loss(const matrix &x, const matrix &y);
    void MSE_grad(const matrix &x, const matrix &y, matrix &grad_out);

    void SGD_step(model_training &m, size_t batch_size, float lr = 0.1, float momentum = 0.9, float weight_decay = 0);
    void zero_grad(model_training &m);
}