SHELL = /bin/bash

CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -DNDEBUG

OBJ_GNN_VC = main.o graph.o local_search.o chils.o
OBJ_GNN_VC := $(addprefix bin/, $(OBJ_CHILS))

P_DYN2 = other_solvers/DynWVC2/
P_FAST = other_solvers/FastWVC/
P_NUMW = other_solvers/NuMWVC/
P_HILS = other_solvers/HILS/

all : DynWVC2 FastWVC NuMWVC HILS GNN_VC

DynWVC2 : $(P_DYN2)mwvc.cpp $(P_DYN2)mwvc.h
	$(CXX) $(CXXFLAGS) -o $@ $(P_DYN2)mwvc.cpp

FastWVC : $(P_FAST)mwvc.cpp $(P_FAST)mwvc.h
	$(CXX) $(CXXFLAGS) -o $@ $(P_FAST)mwvc.cpp

NuMWVC : $(P_NUMW)wvcp_0123.c $(P_NUMW)wvcp_0123.h
	$(CXX) $(CXXFLAGS) -o $@ $(P_NUMW)wvcp_0123.c

HILS : $(P_HILS)main.cpp $(P_HILS)ArgPack.cpp $(P_HILS)bossa_timer.cpp $(P_HILS)Graph.cpp $(P_HILS)Solution.cpp \
	   $(P_HILS)InitError.h $(P_HILS)ArgPack.h $(P_HILS)bossa_timer.h $(P_HILS)Graph.h $(P_HILS)Solution.h
	$(CXX) $(CXXFLAGS) -o $@ $(P_HILS)main.cpp $(P_HILS)ArgPack.cpp $(P_HILS)bossa_timer.cpp $(P_HILS)Graph.cpp $(P_HILS)Solution.cpp

GNN_VC : src/GNN_VC.cpp src/matrix.cpp src/gnn_inference.cpp \
		 include/gnn_inference.hpp include/local_search.hpp include/mwvc_reductions.hpp
	$(CXX) $(CXXFLAGS) -I include -o $@ src/GNN_VC.cpp src/matrix.cpp src/gnn_inference.cpp -lopenblas

.PHONY : clean
clean :
	rm -f DynWVC2 FastWVC NuMWVC HILS GNN_VC