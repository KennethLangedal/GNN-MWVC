#include <iostream>

using namespace std;

int main() {
    size_t E, N, w;
    cin >> E >> N;
    cout << "p edge " << N << " " << E << endl;
    for (size_t i = 0; i < N; ++i) {
        cin >> w;
        cout << "v " << i + 1 << " " << w << endl;
    }
    size_t u, v;
    for (size_t i = 0; i < E; ++i) {
        cin >> u >> v;
        cout << "e " << u << " " << v << endl;
    }
}