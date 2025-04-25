#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <limits.h>
#include <float.h>
#include <memory.h>
#include <sys/times.h>
#include <unistd.h>
#include <math.h>
#include <set>
#include <algorithm>
#include <utility>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#define MAXV 9000000
#define MAXE 80000000

using namespace std;

typedef struct Column
{
	int score;
	int degree;
	long long time_stamp;
	char config;
	char is_in_c;
	char must_in;
	int cost;
} Column;

Column cs[MAXV];
int weight[MAXE];
int e_num, v_num;
int seed;
int BEST;
long long step;
long long total_step;
int best_array[MAXV];
int best_value;
set<int> tabu_list;

int **e_vertex, **v_edges;
int **v_adj;

int *nrow;
int *k, *best_sol;
double start_time;
double best_comp_time;
struct tms start, _end;

int check();
void init();
void init_best();
void init_fast();
void localsearch(int);
void add(int);
void remove(int);
int find_best_in_c(int);
void uncov_r_weight_inc();
void update_best_sol();
int fitness();
void free_all();

void free_all()
{
	free(e_vertex);
	free(v_edges);
	free(v_adj);
	// free(ncol);
	free(nrow);
	free(best_sol);
}
void build_instance2(char *file)
{
	int i, j, h, t;
	ifstream infile(file);
	// FILE *f = freopen(file, "r", stdin);
	string line;

	getline(infile, line);
	stringstream ss(line);
	ss >> v_num >> e_num;
	// scanf("%d %d", &e_num, &v_num);
	// printf("%s e_num=%d v_num=%d\n",file,e_num,v_num);

	if (v_num > MAXV)
	{
		printf("the number of vertices %d exceeds MAXV %d \n", v_num, MAXV);
		exit(0);
	}
	if (e_num > MAXE)
	{
		printf("the number of edges %d exceeds MAXE %d \n", e_num, MAXE);
		exit(0);
	}

	e_vertex = (int **)malloc(e_num * sizeof(int *));
	// ncol=(int *)malloc(m*sizeof(int));

	v_edges = (int **)malloc(v_num * sizeof(int *));
	v_adj = (int **)malloc(v_num * sizeof(int *));
	nrow = (int *)malloc(v_num * sizeof(int));
	memset(nrow, 0, v_num * sizeof(int));
	k = (int *)malloc(v_num * sizeof(int));
	best_sol = (int *)malloc(v_num * sizeof(int));
	
	i = 0;
	for (j = 0; j < v_num; j++)
	{
		getline(infile, line);

		stringstream ss(line);
		ss >> cs[j].cost;

		// scanf("%d", &cs[j].cost);

		cs[j].config = 1;
		cs[j].time_stamp = 1;
		cs[j].is_in_c = 0;
		cs[j].must_in = 0;
		best_sol[j] = 0;
		k[j] = 0;

		int _e;
		while (ss >> _e)
		{
			_e--;
            if (_e <= j)
                continue;
			
			e_vertex[i] = (int *)malloc(2 * sizeof(int));
			e_vertex[i][0] = j;
			e_vertex[i][1] = _e;
			weight[i] = 1;
			i++;
		}
	}
	// for (i = 0; i < e_num; i++)
	// {
	// 	// scanf("%d",&ncol[i]);
	// 	// ncol[i] = 2;
	// 	e_vertex[i] = (int *)malloc(2 * sizeof(int));
	// 	// for(h=0;h<2;h++) {
	// 	scanf("%d %d", &e_vertex[i][0], &e_vertex[i][1]);
	// 	e_vertex[i][0]--;
	// 	e_vertex[i][1]--;
	// 	//}
	// 	weight[i] = 1;
	// }

	// for(j=0;j<v_num;j++) nrow[j]=0;
	for (i = 0; i < e_num; i++)
	{
		// for(h=0;h<2;h++)
		nrow[e_vertex[i][0]]++;
		nrow[e_vertex[i][1]]++;
	}
	for (j = 0; j < v_num; j++)
	{
		v_edges[j] = (int *)malloc(nrow[j] * sizeof(int));
		v_adj[j] = (int *)malloc(nrow[j] * sizeof(int));
		cs[j].score = 0;
		cs[j].degree = nrow[j];
	}
	//	for(i=0;i<e_num;i++) {
	//		for(h=0;h<2;h++) {
	//			col[e_vertex[i][h]][k[e_vertex[i][h]]]=i;
	//			k[e_vertex[i][h]]++;
	//		}
	//	}
	for (i = 0; i < e_num; i++)
	{
		// for(h=0;h<2;h++)

		v_edges[e_vertex[i][0]][k[e_vertex[i][0]]] = i;
		v_edges[e_vertex[i][1]][k[e_vertex[i][1]]] = i;

		v_adj[e_vertex[i][1]][k[e_vertex[i][1]]] = e_vertex[i][0];
		v_adj[e_vertex[i][0]][k[e_vertex[i][0]]] = e_vertex[i][1];

		k[e_vertex[i][0]]++;
		k[e_vertex[i][1]]++;
	}
	free(k);
	// printf("76degree:%d %d %d \n",cs[75].degree,cs[75].cost,cs[27].cost);
}
