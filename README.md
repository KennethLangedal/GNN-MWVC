# GNN-VC &mdash; Efficient Minimum Weight Vertex Cover Heuristics Using Graph Neural Networks

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

By Kenneth Langedal, Johannes Langguth, Fredrik Manne, and Daniel Thilo Schroeder

## Update April 2025

This repository was updated in April 2025 to make the heuristic easier to use and reproduce results from the paper or to compare for further developments of MWVC or MWIS programs. It includes the source code for the following heuristics:
* GNN_VC [Efficient Minimum Weight Vertex Cover Heuristics Using Graph Neural Networks, In Proceedings of the 20th International Symposium on Experimental Algorithms (SEA), 2022]
* NuMWVC [NuMWVC: A Novel Local Search for Minimum Weighted Vertex Cover Problem, In the Journal of the Operational Research Society, 2020]
* FastWVC [Towards Faster Local Search for Minimum Weight Vertex Cover on Massive Graphs, In the Journal of Information Sciences, 2019]
* DynWVC2 [Improving Local Search for Minimum Weight Vertex Cover by Dynamic Strategies, In Proceedings of the 27th International Joint Conference on Artificial Intelligence (IJCAI), 2018]
* HILS [A Hybrid Iterated Local Search Heuristic for the Maximum Weight Independent Set Problem, In the Journal Optimization Letters, 2017]

All the heuristics have been modified to accept the same graph format, as described below. Note that HILS computes an independent set rather than a vertex cover. To compare results, subtract the weight of the independent set from the total sum of weights in the graph.

## Installation

Build all the heuristics by running
```
make
```
This will produce one executable for each heuristic with the respective name. Note that GNN_VC requires OpenBLAS. It can be installed on Linux using:
```
sudo apt-get install libopenblas-dev
```

## Programs

All the programs print a help message if used without any command line arguments. This should make them easy to use without any further documentation. They are as follows
```
./GNN_VC [graph file] [result file] [time] [k (< 0 = auto)] [0 = silent, 1 = verbose]
./NuMWVC [Graph file] [Seed] [Cutoff time]
./FastWVC [Graph file] [Seed] [Cutoff time] [CC mode]
./DynWVC2 [Graph file] [Seed] [Cutoff time] [CC mode]
./HILS [options] <file>
```
For the **GNN_VC** heuristic, the following command was used for the runs in the paper
```
./GNN_VC [graph file] [result file] 1000 -1 0
```
Notice that a result file to store the computed vertex cover is required. The output to **stdout** from running the command above is on the format
```
[graph file],[VC written to file],[Best VC seen],[Time to compute the best VC]
```
Since it can take considerable time to override the best vertex cover in memory continuously, the heuristic only periodically updates the solution that is eventually written to disk. However, it also keeps track of the best vertex cover seen during execution. In some instances, these two numbers can differ significantly.

## Graph Format

All the heuristics expects graphs on the METIS format. A graph with **N** vertices is stored using **N + 1** lines. The first line lists the number of vertices, the number of edges, and the weight type. The first line should use 10 as the weight type to indicate vertex weights. Each subsequent line first gives the weight and then lists the neighbors of that node in **sorted** order.

Here is an example of a graph with 3 vertices of weight 15, 15, and 20, where the weight 20 vertex is connected to the two 15-weight vertices.

```
3 2 10
15 3
15 3
20 1 2
```

Notice that vertices are 1-indexed, and edges appear in the neighborhoods of both endpoints.
