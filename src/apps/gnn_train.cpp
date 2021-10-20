#include "gnn_training.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>

using namespace std;

using Tn = uint32_t;
using Tw = float;

reduction_graph<Tn, Tw> parse_graph(ifstream &fs) {
    uint64_t E, N;
    fs >> E >> N;
    vector<Tw> weights(N);
    for (auto &&w : weights) {
        fs >> w;
        w /= 120.0f;
    }
    vector<pair<Tn, Tn>> edges(E);
    for (auto &&[u, v] : edges) {
        fs >> u >> v;
        if (--u > --v)
            swap(u, v);
    }
    sort(begin(edges), end(edges));

    return reduction_graph<Tn, Tw>(weights, edges);
}

vector<pair<pair<matrix, reduction_graph<Tn, Tw>>, matrix>> load_data(filesystem::path graph_path, filesystem::path labels_path) {
    vector<pair<pair<matrix, reduction_graph<Tn, Tw>>, matrix>> res;

    for (auto &&graph_entry : filesystem::directory_iterator{labels_path}) {
        cout << graph_path / (graph_entry.path().filename().stem().string() + ".mtx") << endl;
        ifstream fs_g(graph_path / (graph_entry.path().filename().stem().string() + ".mtx"));
        auto &&g = parse_graph(fs_g);
        matrix x(g.size(), 1);
        for (Tn u = 0; u < g.size(); ++u) {
            x(u, 0) = g.W(u);
        }
        ifstream fs_l(graph_entry.path());
        matrix y(g.size(), 1);
        float v;
        for (Tn u = 0; u < g.size(); ++u) {
            fs_l >> v;
            y(u, 0) = v;
        }
        
        res.push_back({{x, g}, y});
    }
    return res;
}

struct performance {
    float loss = 0.0f, total_accuracy = 0.0f, num_total = 0.0f, true_accuracy = 0.0f, num_true = 0.0f;
};

ostream &operator<<(ostream &os, const performance &p) {
    os << setw(7) << setfill('0') << p.loss << "\t" << setw(7) << setfill('0') << p.total_accuracy * 100.0f << "\t" << setw(7) << setfill('0') << (size_t)p.num_total << "\t" << setw(7) << setfill('0') << p.true_accuracy * 100.0f << "\t" << setw(7) << setfill('0') << (size_t)p.num_true << "\t";
    return os;
}

template <typename It>
performance run_model(gnn::model_training &m, It first, It last, bool fit = false) {
    performance res;
    while (first != last) {
        auto &&[x, g] = first->first;
        auto &&y = first->second;
        auto &&out = m.predict(x, g);
        for (int i = 0; i < out.get_height(); i++) {
            if (y(i, 0) > 0.5f) {
                res.num_true += 1.0f;
                if (out(i, 0) > 0.5f)
                    res.true_accuracy += 1.0f;
            } else if (out(i, 0) < 0.5f) {
                res.total_accuracy += 1.0f;
            }
        }
        res.loss += gnn::MSE_loss(out, y) * y.get_height();
        res.num_total += y.get_height();
        if (fit) {
            auto &&grad = gnn::MSE_grad(out, y);
            m.backprop(grad, g);
            gnn::SGD_step(m, 0.05f, 0.9f, 0.00001f);
            gnn::zero_grad(m);
        }
        ++first;
    }
    res.loss /= res.num_total;
    res.total_accuracy = (res.true_accuracy + res.total_accuracy) / res.num_total;
    res.true_accuracy /= res.num_true;
    return res;
}

int main() {

    gnn::model_training m("Pure_model");
    size_t seed = 4;
    m.add_layer(gnn::graph_layer_training());
    m.add_layer(gnn::linear_layer_training(5, 16, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(16, 16, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(16, 8, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::graph_layer_training());
    m.add_layer(gnn::linear_layer_training(19, 12, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(12, 12, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(12, 8, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::graph_layer_training());
    m.add_layer(gnn::linear_layer_training(19, 12, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(12, 8, seed));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(8, 1, seed));
    m.add_layer(gnn::sigmoid_training());

    string graph_path, train_label_path, test_label_path;
    cin >> graph_path >> train_label_path >> test_label_path;

    int ne;
    cin >> ne;

    cout << fixed << setprecision(4);

    cout << "Train data: " << endl;
    auto &&train_data = load_data(graph_path, train_label_path);
    cout << "Test data: " << endl;
    auto &&test_data = load_data(graph_path, test_label_path);


    cout << "Epoch\tLoss\t\tAccuracy\tTotal\t\tTrue accuracy\tTrue total\tTest loss\tTest accuracy\tTest total\tTest true acc\tTest true total" << endl;

    for (int e = 0; e <= ne; e++) {
        auto p_train = run_model(m, begin(train_data), end(train_data), true);
        auto p_test = run_model(m, begin(test_data), end(test_data), false);
        if (e % 1 == 0)
            cout << e << "\t" << p_train << p_test << endl;
    }

    cout << m;

    return 0;
}