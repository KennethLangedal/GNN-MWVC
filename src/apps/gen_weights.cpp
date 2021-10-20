#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    string line;
    getline(cin, line);
    while (line.front() == '%')
        getline(cin, line);
    size_t N, E;
    stringstream ss(line);
    ss >> N >> N >> E;
    vector<uint32_t> weights(N);
    mt19937 reng(N);
    uniform_int_distribution<uint32_t> dist(20, 120);
    cout << E << " " << N << endl;
    for (size_t i = 0; i < N; ++i)
        cout << dist(reng) << " ";
    cout << endl;
    uint32_t u, v;
    for (size_t i = 0; i < E; ++i) {
        cin >> u >> v;
        cout << u << " " << v << endl;
    }
    return 0;
}