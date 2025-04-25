/*
 *
 * main.cpp
 *
 *  Created on: 14/09/2015
 *      Author: bruno
 */

#include "Graph.h"
#include "ArgPack.h"
#include "InitError.h"
#include "Solution.h"
#include "bossa_timer.h"

#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <sstream>

using namespace std;
using namespace opt;

namespace opt
{

	// Mersenne Twister 19937 generator

	mt19937 generator;

	/************************************************************
	 *
	 * Creates a graph based on the given specification. Returns
	 * a pointer to the allocated graph; the graph must be
	 * deallocated elsewhere.
	 *
	 ************************************************************/

	Graph *readInstance_old(const string &filename, bool complement)
	{
		int m = -1;		 // number of vertices and edges announced
		int m_count = 0; // number of edges actually counted
		int linenum = 0; // number of readed lines
		// char buffer[256];
		string buffer;
		ifstream input(filename.c_str());
		Graph *graph = NULL;

		if (!input)
		{
			throw InitError("error opening the input file: " + filename + "\n");
		}

		while (getline(input, buffer))
		{
			linenum++;

			int v1, v2, w;
			if (linenum == 2)
			{
				stringstream ss(buffer);
				for (int i = 0; i < graph->n(); ++i)
				{
					ss >> w;
					graph->set_weight(i, w);
				}
				continue;
			}
			if (sscanf(buffer.c_str(), "%d %d", &v2, &v1) != 2)
			{
				input.close();
				throw InitError("syntax error in line " + std::to_string(linenum) + "\n");
			}

			if (linenum == 1)
			{ // read the number of vertices and edges
				if (v1 < 0 || v2 < 0)
				{
					input.close();
					throw InitError("syntax error in line " + std::to_string(linenum) +
									". The number of edges and vertices must not be negative.\n");
				}
				m = v2;
				graph = new Graph(v1, v2);

				if (complement)
				{
					for (int idx1 = 0; idx1 < v1; idx1++)
					{
						for (int idx2 = idx1 + 1; idx2 < v1; idx2++)
						{
							graph->addEdge(idx1, idx2);
						}
					}
				}
			}
			else
			{ // read an edge
				if (v1 < 1 || v2 < 1 || v1 > graph->n() || v2 > graph->n() || v2 == v1)
				{
					input.close();
					cout << "Invalid edge " << v1 << "," << v2 << endl;
					exit(0);
					throw InitError("syntax error in line " + std::to_string(linenum) +
									". Vertices label must not be negative.\n");
				}
				if (!complement)
				{
					graph->addEdge(v1 - 1, v2 - 1);
				}
				else
				{
					graph->removeEdge(v1 - 1, v2 - 1);
				}
				m_count++;
			}
		}

		input.close();

		if (m_count != m)
		{
			throw InitError("the number of edges announced is not equal to the number of edges read.\n");
		}

		return graph;
	} // Graph *readInstance (const string &filename)

	/************************************************************
	 *
	 * Creates a graph based on the given specification. Returns
	 * a pointer to the allocated graph; the graph must be
	 * deallocated elsewhere.
	 *
	 ************************************************************/

	Graph *readInstance(const string &filename, bool complement)
	{
		int m = -1;		 // number of vertices and edges announced
		int m_count = 0; // number of edges actually counted
		int linenum = 0; // number of readed lines
		// char buffer[256];
		string buffer;
		ifstream input(filename.c_str());
		Graph *graph = NULL;

		if (!input)
		{
			throw InitError("error opening the input file: " + filename + "\n");
		}

		while (getline(input, buffer))
		{
			linenum++;
			stringstream ss(buffer);

			if (linenum == 1)
			{
				int n;
				ss >> n >> m;

				graph = new Graph(n, m);

				if (complement)
				{
					for (int idx1 = 0; idx1 < n; idx1++)
					{
						for (int idx2 = idx1 + 1; idx2 < n; idx2++)
						{
							graph->addEdge(idx1, idx2);
						}
					}
				}
			}
			else
			{
				int w, v, u = linenum - 2;
				ss >> w;
				graph->set_weight(u, w);

				while (ss >> v)
				{
					v--;
					if (v <= u)
						continue;

					if (!complement)
					{
						graph->addEdge(u, v);
					}
					else
					{
						graph->removeEdge(u, v);
					}
					m_count++;
				}
			}
		}

		input.close();

		if (m_count != m)
		{
			throw InitError("the number of edges announced is not equal to the number of edges read.\n");
		}

		return graph;
	} // Graph *readInstance (const string &filename)

} // namespace opt

/****************
 *
 * Main function
 *
 ****************/

int main(int argc, char *argv[])
{
	try
	{

		BossaTimer input_timer, proc_timer;
		double target_time = -1;
		// double max_time;
		// cin >> max_time;
		int target_iterations = -1;
		input_timer.start();

		// read input parameters

		ArgPack single_ap(argc, argv);
		double max_time = ArgPack::ap().max_time;

		// set the random seed

		generator.seed(ArgPack::ap().rand_seed);

		// read instance

		Graph *graph_instance = readInstance(ArgPack::ap().input_name, ArgPack::ap().complement);
		input_timer.pause();

		proc_timer.start();
		graph_instance->sort();

		Solution s(graph_instance);

		// randomly initialize a solution

		while (!s.isMaximal())
		{
			s.addRandomVertex();
			assert(s.integrityCheck());
		}

		do
		{
			while (!s.isMaximal())
			{
				s.addRandomVertex();
			}
		} while ((s.omegaImprovement() || s.twoImprovement()) && proc_timer.getTime() < max_time /*|| s.threeImprovement() */);

		Solution best_s(s);
		if (ArgPack::ap().verbose)
			cout << "best weight: " << best_s.weight() << "\n";

		// run ILS iterations

		int k = 1;
		int local_best = s.weight();
		int iter;
		for (iter = 0; iter < ArgPack::ap().iterations && proc_timer.getTime() < max_time; iter++)
		{
			Solution next_s(s);

			// shake
			next_s.force(ArgPack::ap().p[0]);

			assert(next_s.integrityCheck());

			do
			{
				while (!next_s.isMaximal())
				{
					next_s.addRandomVertex();
				}
			} while (next_s.omegaImprovement() || next_s.twoImprovement());

			assert(best_s.integrityCheck());

			if (next_s.weight() > s.weight())
			{
				k = 1;
				s = next_s;

				if (local_best < next_s.weight())
				{
					k -= s.size() / ArgPack::ap().p[1];
					local_best = next_s.weight();
				}

				if (best_s.weight() < s.weight())
				{
					best_s = s;
					k -= s.size() * ArgPack::ap().p[2];

					target_time = proc_timer.getTime();
					target_iterations = iter;

					if (ArgPack::ap().target != 0 && best_s.weight() >= ArgPack::ap().target)
					{
						goto exit;
					}

					if (ArgPack::ap().verbose)
						cout << "new best weight: " << best_s.weight() << " / iteration: " << iter << " / time (s): " << proc_timer.getTime() << "\n";
				}
			}
			else if (k <= s.size() / ArgPack::ap().p[1])
			{
				k++;
			}
			else
			{
				local_best = s.weight();
				s.force(ArgPack::ap().p[3]);
				k = 1;
			}
		}

	exit:
		proc_timer.pause();
		assert(best_s.integrityCheck());

		if (ArgPack::ap().verbose)
		{
			cout << "\n\n- best weight: " << best_s.weight() << "\n";
			cout << "- size: " << best_s.size() << "\n";
			cout << "- solution: ";
			for (int v : best_s.i_set())
			{
				cout << v << " ";
			}
			cout << "\n- input time: " << input_timer.getTime() << "\n";
			cout << "- iterations to find the best: " << target_iterations << "\n";
			cout << "- time to find the best (s): " << target_time << "\n";
			cout << "- total iterations: " << iter << "\n";
			cout << "- total processing time (s): " << proc_timer.getTime() << "\n";
		}
		else
		{
			cout << ArgPack::ap().input_name << "," << (size_t)best_s.weight() << "," << (target_time == -1 ? proc_timer.getTime() : target_time) << "\n";
		}

		delete (graph_instance);
	}
	catch (std::exception &e)
	{
		cerr << e.what();
	}

	return 0;
} // int main(int argc, char *argv[])