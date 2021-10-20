#pragma once
#include "matrix.hpp"
#include "reduction_graph.hpp"
#include <variant>

namespace gnn {
    using Tw = float;
    using Tn = uint32_t;

    struct linear_layer {
        matrix W, bias;

        mutable matrix out;

        linear_layer(size_t dim_in = 0, size_t dim_out = 0, size_t seed = 0);

        const matrix &forward(const matrix &in) const;
    };

    /*
        Performs message passing between nodes.
        Output dim = Input dim * 2 + 3. Where the output looks like:
        |Aggregated neighborhood information|Input|Degree|Weight|Neighborhood weight|
    */
    struct graph_layer {
        mutable matrix out;

        const matrix &forward(const matrix &in, const reduction_graph<Tn, Tw> &g) const;
    };

    struct ReLU {
        mutable matrix out;

        const matrix &forward(const matrix &in) const;
    };

    struct sigmoid {
        mutable matrix out;

        const matrix &forward(const matrix &in) const;
    };

    using component = std::variant<linear_layer, graph_layer, ReLU, sigmoid>;

    class model {
    private:
        std::string name;
        std::vector<component> layers;

    public:
        model(std::string name = "");
        void add_layer(const component &c);

        const matrix &predict(const matrix &in, const reduction_graph<Tn, Tw> &g) const;

        friend std::ostream &operator<<(std::ostream &os, const model &m);
        friend std::istream &operator>>(std::istream &is, model &m);
    };

    std::ostream &operator<<(std::ostream &os, const model &m);
    std::istream &operator>>(std::istream &is, model &m);
}