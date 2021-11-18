# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)  
project(GraphNeuralNetworks DESCRIPTION "GNN & MWVC" VERSION 0.0.1)

# before executing do 
# module load gcc/9.3.0
# module load cmake/gcc/3.21.2
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER gcc-9)
set(CMAKE_CXX_COMPILER g++-9)
set(CMAKE_CXX_FLAGS "-fopenmp")
#set(CMAKE_CXX_FLAGS "-pg")

find_package(OpenBLAS CONFIG REQUIRED)
find_package(OpenMP)

include_directories(include/gnn/)
include_directories(include/mwvc/)

add_executable(GNN_VC_experimental src/apps/GNN_VC_experimental.cpp src/lib/matrix.cpp src/lib/gnn_inference.cpp)
target_link_libraries(GNN_VC_experimental PRIVATE OpenBLAS::OpenBLAS)

add_executable(GNN_VC src/apps/GNN_VC.cpp src/lib/matrix.cpp src/lib/gnn_inference.cpp)
target_link_libraries(GNN_VC PRIVATE OpenBLAS::OpenBLAS)

add_executable(gen_reduced_graph src/apps/gen_reduced_graph.cpp)

add_executable(gen_weights src/apps/gen_weights.cpp)

add_executable(mtx_to_graph src/apps/mtx_to_graph.cpp)

add_executable(approximation_solver src/apps/approximation_solver.cpp)

add_executable(is_vc_converter src/apps/is_vc_converter.cpp)

add_executable(gnn_training src/apps/gnn_train.cpp src/lib/matrix.cpp src/lib/gnn_inference.cpp src/lib/gnn_training.cpp)
target_link_libraries(gnn_training PRIVATE OpenBLAS::OpenBLAS)