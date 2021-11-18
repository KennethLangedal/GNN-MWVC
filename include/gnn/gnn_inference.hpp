#pragma once
#include "matrix.hpp"
#include "reduction_graph.hpp"
#include <variant>

namespace gnn {
    using Tw = uint32_t;
    using Tn = uint32_t;

    struct linear_layer {
        matrix W, bias;

        linear_layer(size_t dim_in = 0, size_t dim_out = 0, size_t seed = 0);

        void forward(const matrix &in, matrix &out) const;
    };

    /*
        Performs message passing between nodes.
        Output dim = Input dim * 2 + 3. Where the output looks like:
        |Aggregated neighborhood information|Input|Degree|Weight|Neighborhood weight|
    */
    struct graph_layer {
        float WEIGHT_SCALE = 120.0f;

        void forward(const matrix &in, matrix &out, const reduction_graph<Tn, Tw> &g) const;
    };

    struct ReLU {
        void forward(const matrix &in, matrix &out) const;
    };

    struct sigmoid {
        void forward(const matrix &in, matrix &out) const;
    };

    using component = std::variant<linear_layer, graph_layer, ReLU, sigmoid>;

    class model {
    private:
        std::string name;
        std::vector<component> layers;
        mutable matrix in_copy;

    public:
        model(std::string name = "");
        void add_layer(const component &c);

        void predict(const matrix &in, matrix &out, const reduction_graph<Tn, Tw> &g) const;

        friend std::ostream &operator<<(std::ostream &os, const model &m);
        friend std::istream &operator>>(std::istream &is, model &m);
    };

    std::ostream &operator<<(std::ostream &os, const model &m);
    std::istream &operator>>(std::istream &is, model &m);
}