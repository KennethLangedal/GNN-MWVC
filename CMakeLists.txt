# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)  
project(GraphNeuralNetworks DESCRIPTION "GNN & MWVC" VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-pg")

find_package(OpenBLAS CONFIG REQUIRED)

include_directories(include/gnn/)
include_directories(include/mwvc/)

add_executable(gen_weights src/apps/gen_weights.cpp)

add_executable(mtx_to_graph src/apps/mtx_to_graph.cpp)

add_executable(is_vc_converter src/apps/is_vc_converter.cpp)

add_executable(gnn_training src/apps/gnn_train.cpp src/lib/matrix.cpp src/lib/gnn_inference.cpp src/lib/gnn_training.cpp)
target_link_libraries(gnn_training PRIVATE OpenBLAS::OpenBLAS)

add_executable(gnn_test src/apps/gnn_test.cpp src/lib/matrix.cpp src/lib/gnn_inference.cpp src/lib/gnn_training.cpp)
target_link_libraries(gnn_test PRIVATE OpenBLAS::OpenBLAS)