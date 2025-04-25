#include "gnn_training.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>

using namespace std;

using Tn = uint32_t;
using Tw = uint32_t;

float WEIGHT_SCALE = 2000.0f;

reduction_graph<Tn, Tw> parse_graph(ifstream &fs) {
    uint64_t E, N;
    fs >> E >> N;
    vector<Tw> weights(N);
    for (auto &&w : weights)
        fs >> w;

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
        // cout << graph_path / (graph_entry.path().filename().stem().string() + ".mtx") << endl;
        ifstream fs_g(graph_path / (graph_entry.path().filename().stem().string() + ".mtx"));
        auto &&g = parse_graph(fs_g);
        matrix x(g.size(), 1);
        for (Tn u = 0; u < g.size(); ++u) {
            x(u, 0) = (float)g.W(u) / WEIGHT_SCALE;
        }
        ifstream fs_l(graph_entry.path());
        matrix y(g.size(), 1);
        float v;
        size_t tc = 0, fc = 0;
        for (Tn u = 0; u < g.size(); ++u) {
            fs_l >> v;
            if (v < 0.5f)
                fc++;
            else
                tc++;
            y(u, 0) = v;
        }

        if (tc > g.size() * 0.2f && fc > g.size() * 0.2f)
            res.push_back({{x, g}, y});
    }
    return res;
}

struct performance {
    float loss = 0.0f, total_accuracy = 0.0f, num_total = 0.0f, true_accuracy = 0.0f, num_true = 0.0f;
};

ostream &operator<<(ostream &os, const performance &p) {
    os << p.loss << "," << p.total_accuracy * 100.0f << "," << (size_t)p.num_total << "," << p.true_accuracy * 100.0f << "," << (size_t)p.num_true << ",";
    return os;
}

template <typename It>
performance run_model(gnn::model_training &m, It first, It last, bool fit = false, size_t batch_size = 500000, float lr = 0.01f, float momentum = 0.9f, float wd = 0.0000f) {
    performance res;
    int t = 0;
    matrix out, grad;
    while (first != last) {
        auto &&[x, g] = first->first;
        auto &&y = first->second;
        m.predict(x, out, g);
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
            gnn::MSE_grad(out, y, grad);
            m.backprop(grad, out, g);
            if (t > batch_size) {
                gnn::SGD_step(m, t, lr, momentum, wd);
                gnn::zero_grad(m);
                t = 0;
            } else
                t += x.get_height();
        }
        ++first;
    }
    if (t > 0) {
        gnn::SGD_step(m, t, lr, momentum, wd);
        gnn::zero_grad(m);
    }
    res.loss /= res.num_total;
    res.total_accuracy = (res.true_accuracy + res.total_accuracy) / res.num_total;
    res.true_accuracy /= res.num_true;
    return res;
}

int main(int narg, char **arg) {
    if (narg != 6) {
        cout << "Usage: ./gnn_train [graph path] [label path] [out path] [number of epochs] [seed]" << endl;
        return 0;
    }

    string graph_path(arg[1]), label_path(arg[2]), out_path(arg[3]);
    size_t ne = stoi(arg[4]);
    ofstream os(out_path);
    if (!os.is_open()) {
        cout << "Unable to open file at out path" << endl;
        return 0;
    }

    gnn::model_training m("MWVC_Model");
    size_t seed = stoi(arg[5]);
    m.add_layer(gnn::graph_layer_training(WEIGHT_SCALE));
    m.add_layer(gnn::linear_layer_training(5, 32, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(32, 32, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(32, 16, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::graph_layer_training(WEIGHT_SCALE));
    m.add_layer(gnn::linear_layer_training(35, 32, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(32, 32, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(32, 16, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::graph_layer_training(WEIGHT_SCALE));
    m.add_layer(gnn::linear_layer_training(35, 32, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(32, 16, seed++));
    m.add_layer(gnn::ReLU_training());
    m.add_layer(gnn::linear_layer_training(16, 1, seed++));
    m.add_layer(gnn::sigmoid_training());

    cout << fixed << setprecision(4);

    mt19937 reng(seed);

    auto train_data = load_data(graph_path, label_path);
    vector<pair<pair<matrix, reduction_graph<Tn, Tw>>, matrix>> test_data;
    shuffle(begin(train_data), end(train_data), reng);
    size_t train_split = train_data.size() * 0.9;
    copy(begin(train_data) + train_split, end(train_data), back_inserter(test_data));
    train_data.erase(begin(train_data) + train_split, end(train_data));

    cout << "Training graphs: " << train_data.size() << ", Test graphs: " << test_data.size() << endl;

    cout << "Epoch,Loss,Accuracy,Total,True accuracy,True total,Test loss,Test accuracy,Test total,Test true acc,Test true total" << endl;

    for (int e = 0; e <= ne; e++) {
        shuffle(begin(train_data), end(train_data), reng);
        auto p_train = run_model(m, begin(train_data), end(train_data), true);
        auto p_test = run_model(m, begin(test_data), end(test_data), false);
        if (e % 1 == 0)
            cout << e << "," << p_train << p_test << endl;
    }

    os << m;

    return 0;
}