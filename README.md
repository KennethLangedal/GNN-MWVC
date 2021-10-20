# Graph Neural Networks

Using CMake and OpenBLAS

## Programs

### gnn_train

> ./gnn_train

Input via cin:
* Path to graphs.
* Path to training labels.
* Path to test labels.
* The number of epochs.

All the paths should be folders.

Outputs training information and then the model via cout.

### gnn_test

> ./gnn_test [model] [graph] (optional [solution])

### gen_wieghts

> ./gen_weights < graph.mtx > weighted_graph.mtxt

Input the graph via cin. It assigns weights in the range [20,120] using N as seed. The input can be the typical .mtx format from SuiteSparse. The output also has the initial comments removed.

### mtx_to_graph

> ./mtx_to_graph < name.mtx > name.graph

Converts weighted mtx graph to the format KaMWIS requires.

### is_vc_converter

> ./is_vc_converter [graph] [solution] [output]

Takes the graph as .mtx, the solution file can come directly from KaMWIS (one 0 or 1 per line, N lines).

The output file will have the numbers flipped. The program also tests if the vertex cover is valid and reports the cost.