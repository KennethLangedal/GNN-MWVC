#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <string>
#include <algorithm>
#include <random>
#include <cmath>
#include <cstring>
#include <sstream>

using namespace std;

#define pop(stack) stack[--stack##_fill_pointer]
#define push(item, stack) stack[stack##_fill_pointer++] = item

typedef long long llong;
typedef unsigned int uint;
struct Edge
{
    int v1;
    int v2;
};

chrono::steady_clock::time_point start;
llong max_steps;
llong step;
int try_step;
uint seed;
int cutoff_time;
int mode;

int v_num;
int e_num;

Edge *edge;
int *edge_weight;

int *dscore;
llong *time_stamp;
int *valid_score;

int *v_weight;
int **v_edges;
int **v_adj;
int *v_degree;

int c_size;
int *v_in_c;
int *remove_cand;
int *index_in_remove_cand;
int remove_cand_size;
llong now_weight;

int best_c_size;
int *best_v_in_c;
double best_comp_time;
llong best_step;
llong best_weight;

int *uncov_stack;
int uncov_stack_fill_pointer;
int *index_in_uncov_stack;

int ave_weight;
int delta_total_weight;
int threshold;
double p_scale;

int BuildInstance(string);
void FreeMemory();
void ResetRemoveCand();
inline void Uncover(int);
inline void Cover(int);
void Add(int);
void Remove(int);
int UpdateTargetSize();
int ChooseRemoveV1();
int ChooseRemoveV2();
int ChooseAddV(int, int, int);
void UpdateBestSolution();
void RemoveRedundant();
void ConstructVC();
int CheckSolution();
// void ForgetEdgeWeights();
void UpdateEdgeWeight();
void LocalSearch();
double TimeElapsed();

double TimeElapsed()
{
    chrono::steady_clock::time_point finish = chrono::steady_clock::now();
    chrono::duration<double> duration = finish - start;
    return duration.count();
}

int BuildInstance(string filename)
{
    string line;
    int u, v, e;
    int v1, v2;

    ifstream infile(filename);
    if (!infile)
    {
        return 1;
    }

    getline(infile, line);
    stringstream ss(line);
    ss >> v_num >> e_num;

    edge = new Edge[e_num];
    edge_weight = new int[e_num];
    uncov_stack = new int[e_num];
    index_in_uncov_stack = new int[e_num];
    dscore = new int[v_num + 1];
    valid_score = new int[v_num + 1];
    time_stamp = new llong[v_num + 1];
    v_edges = new int *[v_num + 1];
    v_adj = new int *[v_num + 1];
    v_degree = new int[v_num + 1];
    v_weight = new int[v_num + 1];
    v_in_c = new int[v_num + 1];
    remove_cand = new int[v_num + 1];
    index_in_remove_cand = new int[v_num + 1];
    best_v_in_c = new int[v_num + 1];

    fill_n(v_degree, v_num + 1, 0);
    fill_n(v_in_c, v_num + 1, 0);
    fill_n(dscore, v_num + 1, 0);
    fill_n(time_stamp, v_num + 1, 0);
    fill_n(edge_weight, e_num, 1);
    fill_n(valid_score, v_num + 1, 1000000);

    e = 0;
    for (v = 1; v < v_num + 1; v++)
    {
        getline(infile, line);

        stringstream ss(line);
        ss >> v_weight[v];

        while (ss >> u)
        {
            if (u <= v)
                continue;

            v_degree[v]++;
            v_degree[u]++;

            edge[e].v1 = v;
            edge[e].v2 = u;
            e++;
        }
    }

    // for (e = 0; e < e_num; e++)
    // {
    //     infile >> tmp >> v1 >> v2;
    //     if (v1 < 1 || v1 > v_num || v2 < 1 || v2 > v_num || v1 == v2)
    //     {
    //         cout << "Invalid edge " << v1 << "," << v2 << endl;
    //         exit(0);
    //     }
    //     v_degree[v1]++;
    //     v_degree[v2]++;

    //     edge[e].v1 = v1;
    //     edge[e].v2 = v2;
    // }
    infile.close();

    if (e != e_num)
    {
        cout << "Invalid number of edges, " << e << " vs " << e_num << endl;
        exit(0);
    }

    v_adj[0] = 0;
    v_edges[0] = 0;
    for (v = 1; v < v_num + 1; v++)
    {
        v_adj[v] = new int[v_degree[v]];
        v_edges[v] = new int[v_degree[v]];
    }

    int *v_degree_tmp = new int[v_num + 1];
    fill_n(v_degree_tmp, v_num + 1, 0);

    for (e = 0; e < e_num; e++)
    {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        v_edges[v1][v_degree_tmp[v1]] = e;
        v_edges[v2][v_degree_tmp[v2]] = e;

        v_adj[v1][v_degree_tmp[v1]] = v2;
        v_adj[v2][v_degree_tmp[v2]] = v1;

        v_degree_tmp[v1]++;
        v_degree_tmp[v2]++;
    }
    delete[] v_degree_tmp;

    return 0;
}

void FreeMemory()
{
    int v;
    for (v = 0; v < v_num + 1; v++)
    {
        delete[] v_adj[v];
        delete[] v_edges[v];
    }

    delete[] best_v_in_c;
    delete[] index_in_remove_cand;
    delete[] remove_cand;
    delete[] v_in_c;
    delete[] v_weight;
    delete[] v_degree;
    delete[] v_adj;
    delete[] v_edges;
    delete[] time_stamp;
    delete[] dscore;
    delete[] index_in_uncov_stack;
    delete[] uncov_stack;
    delete[] edge_weight;
    delete[] edge;
    delete[] valid_score;
}

void ResetRemoveCand()
{
    int v, degree, i;
    int j = 0;

    for (v = 1; v < v_num + 1; v++)
    {
        if (v_in_c[v] == 1)
        {
            remove_cand[j] = v;
            index_in_remove_cand[v] = j;
            j++;

            valid_score[v] = -v_weight[v];
            degree = v_degree[v];
            for (i = 0; i < degree; i++)
            {
                if (v_in_c[v_adj[v][i]] == 0)
                {
                    valid_score[v] += v_weight[v_adj[v][i]];
                }
            }
        }
        else
        {
            index_in_remove_cand[v] = 0;
        }
    }

    remove_cand_size = j;
}

inline void Uncover(int e)
{
    index_in_uncov_stack[e] = uncov_stack_fill_pointer;
    push(e, uncov_stack);
}

inline void Cover(int e)
{
    int index, last_uncov_edge;
    last_uncov_edge = pop(uncov_stack);
    index = index_in_uncov_stack[e];
    uncov_stack[index] = last_uncov_edge;
    index_in_uncov_stack[last_uncov_edge] = index;
}

void Add(int v)
{
    int i, e, n;
    int edge_count = v_degree[v];

    v_in_c[v] = 1;
    c_size++;
    dscore[v] = -dscore[v];
    now_weight += v_weight[v];
    valid_score[v] = -v_weight[v];

    remove_cand[remove_cand_size] = v;
    index_in_remove_cand[v] = remove_cand_size++;

    for (i = 0; i < edge_count; i++)
    {
        e = v_edges[v][i];
        n = v_adj[v][i];

        if (v_in_c[n] == 0)
        {
            dscore[n] -= edge_weight[e];
            Cover(e);
            valid_score[v] += v_weight[n];
        }
        else
        {
            dscore[n] += edge_weight[e];
            valid_score[n] -= v_weight[v];
            if (valid_score[n] == -v_weight[n])
            {
                Remove(n);
            }
        }
    }
}

void Remove(int v)
{
    int i, e, n;
    int edge_count = v_degree[v];

    v_in_c[v] = 0;
    c_size--;
    dscore[v] = -dscore[v];
    valid_score[v] = 1000000;

    int last_remove_cand_v = remove_cand[--remove_cand_size];
    int index = index_in_remove_cand[v];
    remove_cand[index] = last_remove_cand_v;
    index_in_remove_cand[last_remove_cand_v] = index;
    index_in_remove_cand[v] = 0;

    now_weight -= v_weight[v];

    for (i = 0; i < edge_count; i++)
    {
        e = v_edges[v][i];
        n = v_adj[v][i];

        if (v_in_c[n] == 0)
        {
            dscore[n] += edge_weight[e];
            Uncover(e);
        }
        else
        {
            dscore[n] -= edge_weight[e];
            valid_score[n] += v_weight[v];
        }
    }
}

int UpdateTargetSize()
{
    int v;
    int best_remove_v;
    double best_dscore;
    double dscore_v;

    best_remove_v = remove_cand[0];
    best_dscore = (double)(v_weight[best_remove_v]) / (double)abs(dscore[best_remove_v]);

    if (dscore[best_remove_v] != 0)
    {
        for (int i = 1; i < remove_cand_size; i++)
        {
            v = remove_cand[i];
            if (dscore[v] == 0)
                break;
            dscore_v = (double)(v_weight[v]) / (double)abs(dscore[v]);
            if (dscore_v > best_dscore)
            {
                best_dscore = dscore_v;
                best_remove_v = v;
            }
        }
    }
    Remove(best_remove_v);
    return best_remove_v;
}

int ChooseRemoveV1()
{
    int i, v;
    int remove_v = remove_cand[0];
    int improvement_remove = valid_score[remove_v], improvement_v;
    for (i = 1; i < remove_cand_size; i++)
    {
        v = remove_cand[i];
        improvement_v = valid_score[v];
        if (improvement_v > improvement_remove)
        {
            continue;
        }
        if (improvement_v < improvement_remove)
        {
            remove_v = v;
            improvement_remove = improvement_v;
        }
        else if (time_stamp[v] < time_stamp[remove_v])
        {
            remove_v = v;
            improvement_remove = improvement_v;
        }
    }
    return remove_v;
}

int ChooseRemoveV2()
{
    int i, v;
    double dscore_v, dscore_remove_v;
    int remove_v = remove_cand[rand() % remove_cand_size];
    int to_try = 50;

    for (i = 1; i < to_try; i++)
    {
        v = remove_cand[rand() % remove_cand_size];
        dscore_v = (double)v_weight[v] / (double)abs(dscore[v]);
        dscore_remove_v = (double)v_weight[remove_v] / (double)abs(dscore[remove_v]);
        if (dscore_v < dscore_remove_v)
        {
            continue;
        }
        if (dscore_v > dscore_remove_v)
        {
            remove_v = v;
        }
        else if (time_stamp[v] < time_stamp[remove_v])
        {
            remove_v = v;
        }
    }
    return remove_v;
}

int ChooseAddFromV()
{
    int v;
    int add_v = 0;
    double improvemnt = 0.0;
    double dscore_v;

    for (v = 1; v < v_num + 1; v++)
    {
        if (v_in_c[v] == 1)
        {
            continue;
        }
        dscore_v = (double)dscore[v] / (double)(v_weight[v]);
        if (dscore_v > improvemnt)
        {
            improvemnt = dscore_v;
            add_v = v;
        }
        else if (dscore_v == improvemnt)
        {
            if (time_stamp[v] < time_stamp[add_v])
            {
                add_v = v;
            }
        }
    }
    return add_v;
}

int ChooseAddV(int update_v, int remove_v = 0, int remove_v2 = 0)
{
    int i, v;
    int add_v = 0;
    double improvemnt = 0.0;
    double dscore_v;

    int tmp_degree = v_degree[update_v];
    int *adjp = v_adj[update_v];

    for (i = 0; i < tmp_degree; i++)
    {
        v = adjp[i];
        if (v_in_c[v] == 1)
        {
            continue;
        }
        dscore_v = (double)dscore[v] / (double)(v_weight[v]);
        if (dscore_v > improvemnt)
        {
            improvemnt = dscore_v;
            add_v = v;
        }
        else if (dscore_v == improvemnt)
        {
            if (time_stamp[v] < time_stamp[add_v])
            {
                add_v = v;
            }
        }
    }

    if (remove_v != 0)
    {
        tmp_degree = v_degree[remove_v];
        adjp = v_adj[remove_v];
        for (i = 0; i < tmp_degree; i++)
        {
            v = adjp[i];
            if (v_in_c[v] == 1)
            {
                continue;
            }
            dscore_v = (double)dscore[v] / (double)(v_weight[v]);
            if (dscore_v > improvemnt)
            {
                improvemnt = dscore_v;
                add_v = v;
            }
            else if (dscore_v == improvemnt)
            {
                if (time_stamp[v] < time_stamp[add_v])
                {
                    add_v = v;
                }
            }
        }
    }

    if (remove_v2 != 0)
    {
        tmp_degree = v_degree[remove_v2];
        adjp = v_adj[remove_v2];
        for (i = 0; i < tmp_degree; i++)
        {
            v = adjp[i];
            if (v_in_c[v] == 1)
            {
                continue;
            }
            dscore_v = (double)dscore[v] / (double)(v_weight[v]);
            if (dscore_v > improvemnt)
            {
                improvemnt = dscore_v;
                add_v = v;
            }
            else if (dscore_v == improvemnt)
            {
                if (time_stamp[v] < time_stamp[add_v])
                {
                    add_v = v;
                }
            }
        }
    }

    return add_v;
}

void UpdateBestSolution()
{
    int v;

    if (now_weight < best_weight)
    {
        for (v = 1; v < v_num + 1; v++)
        {
            best_v_in_c[v] = v_in_c[v];
        }
        best_weight = now_weight;
        best_c_size = c_size;
        best_comp_time = TimeElapsed();
        best_step = step;
    }
}

void RemoveRedundant()
{
    int v;
    for (int i = 0; i < remove_cand_size; i++)
    {
        v = remove_cand[i];
        if (v_in_c[v] == 1 && dscore[v] == 0)
        {
            Remove(v);
            i--;
        }
    }
}

void ConstructVC()
{
    int e;
    int v1, v2;
    double v1dd, v2dd;

    uncov_stack_fill_pointer = 0;
    c_size = 0;
    best_weight = (int)(~0U >> 1);
    now_weight = 0;

    for (e = 0; e < e_num; e++)
    {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        if (v_in_c[v1] == 0 && v_in_c[v2] == 0)
        {
            v1dd = (double)v_degree[v1] / (double)v_weight[v1];
            v2dd = (double)v_degree[v2] / (double)v_weight[v2];
            if (v1dd > v2dd)
            {
                v_in_c[v1] = 1;
                now_weight += v_weight[v1];
            }
            else
            {
                v_in_c[v2] = 1;
                now_weight += v_weight[v2];
            }
            c_size++;
        }
    }

    int *save_v_in_c = new int[v_num + 1];
    memcpy(save_v_in_c, v_in_c, sizeof(int) * (v_num + 1));
    int save_c_size = c_size;
    llong save_weight = now_weight;

    int times = 50;
    vector<int> blocks(e_num / 1024 + 1);
    for (int i = 0; i < e_num / 1024 + 1; i++)
    {
        blocks[i] = i;
    }

    while (times-- > 0)
    {
        fill_n(v_in_c, v_num + 1, 0);
        c_size = 0;
        now_weight = 0;
        shuffle(blocks.begin(), blocks.end(), default_random_engine(seed));

        for (auto &block : blocks)
        {
            auto begin = block * 1024;
            auto end = block == e_num / 1024 ? e_num : begin + 1024;
            int tmpsize = end - begin + 1;
            vector<int> idx(tmpsize);
            for (int i = begin; i < end; i++)
            {
                idx[i - begin] = i;
            }
            while (tmpsize > 0)
            {
                int i = rand() % tmpsize;
                Edge e = edge[idx[i]];
                v1 = e.v1;
                v2 = e.v2;
                swap(idx[i], idx[--tmpsize]);
                if (v_in_c[v1] == 0 && v_in_c[v2] == 0)
                {
                    v1dd = (double)v_degree[v1] / (double)v_weight[v1];
                    v2dd = (double)v_degree[v2] / (double)v_weight[v2];
                    if (v1dd > v2dd)
                    {
                        v_in_c[v1] = 1;
                        now_weight += v_weight[v1];
                    }
                    else
                    {
                        v_in_c[v2] = 1;
                        now_weight += v_weight[v2];
                    }
                    c_size++;
                }
            }
        }
        if (now_weight < save_weight)
        {
            save_weight = now_weight;
            save_c_size = c_size;
            memcpy(save_v_in_c, v_in_c, sizeof(int) * (v_num + 1));
        }
    }

    now_weight = save_weight;
    c_size = save_c_size;
    memcpy(v_in_c, save_v_in_c, sizeof(int) * (v_num + 1));
    delete[] save_v_in_c;

    for (e = 0; e < e_num; e++)
    {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        if (v_in_c[v1] == 1 && v_in_c[v2] == 0)
        {
            dscore[v1] -= edge_weight[e];
        }
        else if (v_in_c[v2] == 1 && v_in_c[v1] == 0)
        {
            dscore[v2] -= edge_weight[e];
        }
    }

    ResetRemoveCand();
    for (int v = 1; v < v_num + 1; v++)
    {
        if (v_in_c[v] == 1 && dscore[v] == 0)
        {
            Remove(v);
        }
    }
    UpdateBestSolution();
}

int CheckSolution()
{
    int e, v;

    for (e = 0; e < e_num; ++e)
    {
        if (best_v_in_c[edge[e].v1] != 1 && best_v_in_c[edge[e].v2] != 1)
        {
            cout << ", uncovered edge " << e;
            return 0;
        }
    }
    return 1;
}

void ForgetEdgeWeights()
{
    int v, e;
    int new_total_weitght = 0;

    for (v = 1; v < v_num + 1; v++)
    {
        dscore[v] = 0;
    }

    for (e = 0; e < e_num; e++)
    {
        edge_weight[e] = edge_weight[e] * p_scale;
        new_total_weitght += edge_weight[e];

        if (v_in_c[edge[e].v1] + v_in_c[edge[e].v2] == 0)
        {
            dscore[edge[e].v1] += edge_weight[e];
            dscore[edge[e].v2] += edge_weight[e];
        }
        else if (v_in_c[edge[e].v1] + v_in_c[edge[e].v2] == 1)
        {
            if (v_in_c[edge[e].v1] == 1)
            {
                dscore[edge[e].v1] -= edge_weight[e];
            }
            else
            {
                dscore[edge[e].v2] -= edge_weight[e];
            }
        }
    }
    ave_weight = new_total_weitght / e_num;
}

void UpdateEdgeWeight()
{
    int i, e;

    for (i = 0; i < uncov_stack_fill_pointer; i++)
    {
        e = uncov_stack[i];
        edge_weight[e] += 1;
        dscore[edge[e].v1] += 1;
        dscore[edge[e].v2] += 1;
    }

    delta_total_weight += uncov_stack_fill_pointer;

    if (mode / 2 == 1)
    {
        if (delta_total_weight >= e_num)
        {
            ave_weight += 1;
            delta_total_weight -= e_num;
        }

        if (ave_weight >= threshold)
        {
            ForgetEdgeWeights();
        }
    }
}

void LocalSearch()
{
    int add_v, remove_v = 0, update_v = 0, remove_v2 = 0;
    int noimprovement = 0, dyn_count = 0, temp_weight;
    step = 1;
    try_step = 100;
    int remove_degree = 0;

    ave_weight = 1;
    delta_total_weight = 0;
    p_scale = 0.3;
    threshold = (int)(0.5 * v_num);
    while (true)
    {
        temp_weight = now_weight;
        UpdateBestSolution();
        update_v = UpdateTargetSize();
        time_stamp[update_v] = step;
        if (step % try_step == 0)
        {
            if (TimeElapsed() >= cutoff_time)
            {
                return;
            }
        }
        if (noimprovement < 5)
        {
            remove_v = ChooseRemoveV1();
            Remove(remove_v);
            time_stamp[remove_v] = step;
        }
        else
        {
            if (noimprovement == 5)
            {
                dyn_count = 2;
            }
            if (dyn_count == 1)
            {
                noimprovement = 0;
            }
            remove_v = ChooseRemoveV2();
            Remove(remove_v);
            time_stamp[remove_v] = step;
            dyn_count--;
        }
        remove_degree = v_degree[update_v] + v_degree[remove_v];
        if (remove_degree < 2 * e_num / v_num)
        {
            remove_v2 = ChooseRemoveV2();
            Remove(remove_v2);
            time_stamp[remove_v2] = step;
        }
        while (uncov_stack_fill_pointer > 0)
        {
            add_v = ChooseAddV(update_v, remove_v, remove_v2);
            Add(add_v);
            UpdateEdgeWeight();
            time_stamp[add_v] = step;
        }
        step++;
        remove_v2 = 0;
        if (now_weight >= temp_weight)
        {
            noimprovement += 1;
        }
        remove_degree = 0;
    }
}
