#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    size_t N, E, w, u, v;

    cin >> E >> N;

    vector<uint32_t> weights(N);
    vector<vector<uint32_t>> edges(E);

    for (size_t i = 0; i < N; ++i) {
        cin >> weights[i];
    }
    for (size_t i = 0; i < E; ++i) {
        cin >> u >> v;
        edges[u - 1].push_back(v);
        edges[v - 1].push_back(u);
    }
    for (size_t i = 0; i < N; ++i) {
        sort(begin(edges[i]), end(edges[i]));
    }
    cout << N << " " << E << " 10" << endl;
    for (size_t i = 0; i < N; ++i) {
        cout << weights[i] << " ";
        for (size_t j = 0; j < edges[i].size(); ++j) {
            cout << edges[i][j] << " ";
        }
        cout << endl;
    }
    return 0;
}