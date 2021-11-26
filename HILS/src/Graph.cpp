#include "Graph.h"

#include <list>
#include <assert.h>
#include <algorithm>

using namespace std;

namespace opt
{

Graph::Graph(const int n, const int m) :
	weights_(n, 0),
	n_(n),
	m_(m),
	adj_l_(n, std::vector<int>())
{
	for(int idx = 0; idx < n; idx++) {
		weights_[idx] = (idx + 1) % 200 + 1;
	}
}

void Graph::removeEdge(const int i, const int j)
{
	assert(i < n_ && j < n_);

	removeNeighbor(i, j);
	removeNeighbor(j, i);
}

void Graph::addEdge(const int i, const int j)
{
	assert(i < n_ && j < n_);

	addNeighbor(i, j);
	addNeighbor(j, i);
}

void Graph::sort()
{
	for (int v = 0; v < n_; v++) {
		std::sort(adj_l_[v].begin(), adj_l_[v].end());
	}
}

} // namespace opt