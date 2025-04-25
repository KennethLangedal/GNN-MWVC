#include "wvcp_0123.h"
int uncover[MAXE];
int index_uncover[MAXE];
// int loss[MAXE];
int uncover_num;
double time_limit;
int cost_C();
bool initTime;
void init()
{
	int i;
	best_value = INT_MAX; // å–æœ€å¤§å�?
	uncover_num = e_num;
	tabu_list.clear(); // ###########
	for (i = 0; i < e_num; i++)
	{
		// if(ncol[i]>0){
		uncover[i] = i;
		index_uncover[i] = i;
		//	}
		//		if(nrow[i]==1) {
		//			cs[col[i][0]].must_in=1;
		//			add(row[i][0]);
		//		}
	}
	// printf("init _end\n");
}
inline int compare(int s1, int c1, int s2, int c2)
{
	if (c1 == c2)
	{
		if (s1 > s2)
			return 1;
		else if (s1 == s2)
			return 0;
		else
			return -1;
	}
	long long t1 = s1, t2 = s2;
	t1 = t1 * c2;
	t2 = t2 * c1;
	if (t1 > t2)
		return 1;
	else if (t1 == t2)
		return 0;
	else
		return -1;
}
void init_best()
{
	int cnt;
	int i, j, k, l, jj;
	int sr, ct;
	while (uncover_num > 0)
	{
		cnt = 0;
		sr = INT_MIN;
		ct = 1;
		for (j = 0; j < v_num; j++)
		{
			if (cs[j].is_in_c)
				continue;
			if (cs[j].must_in)
			{
				continue;
			}
			k = compare(sr, ct, cs[j].score, cs[j].cost);
			if (sr == INT_MIN || k < 0)
			{
				sr = cs[j].score;
				ct = cs[j].cost;
				best_array[0] = j;
				cnt = 1;
			}
			else if (k == 0)
			{
				best_array[cnt++] = j;
			}
		}
		if (cnt > 0)
		{
			l = rand() % cnt;
			add(best_array[l]);
		}
	}
	update_best_sol();
	times(&_end);
	double inittime = double(_end.tms_utime - start.tms_utime + _end.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
	inittime = round(best_comp_time * 100) / 100.0;
	// printf("intial _end time: %lf\n",inittime);
	if (check() == 1)
		printf(" _end ok \n");
	// printf("initial solution:%d\n",best_value);
}
void init_fast()
{
	// times(&_end);
	// best_comp_time = double(_end.tms_utime - start.tms_utime + _end.tms_stime - start.tms_stime)/sysconf(_SC_CLK_TCK);
	// best_comp_time = round(best_comp_time * 100)/100.0;
	// printf("intial begin time: %d\n",best_comp_time);
	int v1, v2;
	int i, j, k, t, cnt, s, ii, jj, ix, h, l;
	for (int e = 0; e < e_num; e++)
	{
		v1 = e_vertex[e][0];
		v2 = e_vertex[e][1];
		// printf("%d %d\n",v1,v2);
		// getchar();getchar();

		index_uncover[e] = -1;
		if (cs[v1].is_in_c == 0 && cs[v2].is_in_c == 0)
		{
			int k = compare(cs[v1].degree, cs[v1].cost, cs[v2].degree, cs[v2].cost);
			if (k > 0)
			{
				// add(v1);
				cs[v1].is_in_c = 1;
				// cs[v1].score=-cs[v1].score£»
			}
			else
			{
				// add(v2);
				cs[v2].is_in_c = 1;
				// cs[v2].score=-cs[v2].score£»
			}
		}
	}
	uncover_num = 0;
	// printf("inital first\n");
	// if(check()==1) printf(" first ok \n");
	/*
	for( j=0;j<v_num;j++){

		if(cs[j].is_in_c==0) continue;
		//cs[j].score=0;
		if(cs[j].must_in) {
		//    add(j);
		  cs[j].is_in_c=1;
			continue;
		}
		//loss[j]=0;
	}
	*/

	for (int e = 0; e < e_num; e++)
	{ // ÒòÎª³õÊ¼»¯Ê±´ÓÒ»Ìõ±ßµÄÁ½¸ö¶¥µãÖÐÑ¡Ò»¸ö£¬ËùÒÔÒ»¶¨Ã»ÓÐscore+=weightµÄÇé¿ö
		v1 = e_vertex[e][0];
		v2 = e_vertex[e][1];
		if (cs[v1].is_in_c == 1 && cs[v2].is_in_c == 0)
			// loss[v1]++;
			cs[v1].score--;
		else if (cs[v2].is_in_c == 1 && cs[v1].is_in_c == 0)
			// loss[v2]++;
			cs[v2].score--;
	}

	for (j = 0; j < v_num; j++)
	{
		if (cs[j].score == 0 && cs[j].is_in_c == 1)
		{
			remove(j);
			//            for(int h=0;h<nrow[j];h++){
			//                    if(cs[col[j][h]].is_in_c==1)
			//                loss[col[j][h]]++;
			//            }
		}
	}
	// for(int c=0;c<n;c++){
	// 	if(cs[c].is_in_c){
	// 		for(h=0;h<nrow[c]; h++){//Cé›†åˆä¸­æ¯ä¸€ä¸ªå˜é‡h   å¤„ç†æœªè¦†ç›–é›†å�?
	// 		i=col[c][h];
	// 		if(index_uncover[i]!=-1){
	// 			ii=index_uncover[i];
	// 			for(jj=ii;jj<uncover_num-1;jj++){
	// 				uncover[jj]=uncover[jj+1];
	// 				index_uncover[uncover[jj]]=jj;
	// 			}
	// 			uncover_num--;
	// 			index_uncover[i]=-1;
	// 		}
	// 		// cnt=0;
	// 		// for(l=0;l<ncol[i];l++){//å› ä¸ºè¿™ä¸ªå˜é‡ï¼Œå˜æˆè¿™ä¸ªé›†åˆçš„é‚»å±…é›†å�?
	// 		// 	j=row[i][l];
	// 		// 	if(j==c) continue;
	// 		// 	if(cs[j].is_in_c){
	// 		// 		s=j;
	// 		// 		cnt++;
	// 		// 	}
	// 		// 	cs[j].config=1;
	// 		// }
	// 		// if(cnt==0){ // c is the first one covering this row in C
	// 		// 	for(l=0; l<ncol[i]; l++){
	// 		// 		j=row[i][l];
	// 		// 		if(j==c) continue;
	// 		// 		cs[j].score-=rs[i].weight;  //å€™é€‰è§£ä¸­ä¸è¦†ç›–è¿™ä¸ªå˜é‡ï¼Œæ‰€ä»¥å½“è¦†ç›–ä¹‹åŽï¼Œæ‰€ä»¥ä»¥å‰è¦†ç›–è¿™ä¸ªå˜é‡çš„é›†åˆscoreå–å€¼å¿…é¡»å‡åŽ»è¿™ä¸ªå˜é�?
	// 		// 	}
	// 		// } else if(cnt==1){// c is second one covering this row in C
	// 		// 	cs[s].score+=rs[i].weight; //å€™é€‰è§£ä¸­è¦†ç›–è¿™ä¸ªå˜é‡ä¸€æ¬¡ï¼Œæ‰€ä»¥åŠ å…¥è¿™ä¸ªé›†åˆä»¥åŽï¼Œæ‰€ä»¥ä»¥å‰è¦†ç›–è¿™ä¸ªå˜é‡çš„é›†åˆscoreå–å€¼å¿…é¡»åŠ ä¸Šè¿™ä¸ªå˜é�?
	// 		// }
	// 	}
	// 	}
	// }

	update_best_sol();
	times(&_end);
	double inittime = double(_end.tms_utime - start.tms_utime + _end.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
	inittime = round(best_comp_time * 100) / 100.0;
	// printf("intial _end time: %lf\n",inittime);
	if (check() != 1)
		printf(" _end not ok \n");
	// printf("******************************************\n");
	// printf("init best=--------------: %d \n",best_value);
}

void add(int c)
{
	// printf("a%d\n",c);
	int i, j, k, t, cnt, s, ii, jj, h, l;
	cs[c].is_in_c = 1;
	cs[c].score = -cs[c].score;
	cs[c].config = 0;
	// cs[c].config=1;
	for (h = 0; h < nrow[c]; h++)
	{ // Cé›†åˆä¸­æ¯ä¸€ä¸ªå˜é‡h   å¤„ç†æœªè¦†ç›–é›†å�?
		i = v_edges[c][h];
		/*if(index_uncover[i]!=-1){
			ii=index_uncover[i];
			for(jj=ii;jj<uncover_num-1;jj++){
				uncover[jj]=uncover[jj+1];
				index_uncover[uncover[jj]]=jj;
			}
			uncover_num--;
			index_uncover[i]=-1;
		}*/
		if (index_uncover[i] != -1)
		{
			int last_uncov_edge = uncover[uncover_num - 1];
			int index = index_uncover[i];
			uncover[index] = last_uncov_edge;
			index_uncover[last_uncov_edge] = index;
			index_uncover[i] = -1;
			uncover_num--;
		}
		cnt = 0;
		for (l = 0; l < 2; l++)
		{ // å› ä¸ºè¿™ä¸ªå˜é‡ï¼Œå˜æˆè¿™ä¸ªé›†åˆçš„é‚»å±…é›†å�?
			j = e_vertex[i][l];
			if (j == c)
				continue;
			if (cs[j].is_in_c)
			{
				s = j;
				cnt++;
			}
			cs[j].config = 1;
		}
		if (cnt == 0)
		{ // c is the first one covering this row in C
			for (l = 0; l < 2; l++)
			{
				j = e_vertex[i][l];
				if (j == c)
					continue;
				cs[j].score -= weight[i]; // å€™é€‰è§£ä¸­ä¸è¦†ç›–è¿™ä¸ªå˜é‡ï¼Œæ‰€ä»¥å½“è¦†ç›–ä¹‹åŽï¼Œæ‰€ä»¥ä»¥å‰è¦†ç›–è¿™ä¸ªå˜é‡çš„é›†åˆscoreå–å€¼å¿…é¡»å‡åŽ»è¿™ä¸ªå˜é�?
			}
		}
		else if (cnt == 1)
		{							  // c is second one covering this row in C
			cs[s].score += weight[i]; // å€™é€‰è§£ä¸­è¦†ç›–è¿™ä¸ªå˜é‡ä¸€æ¬¡ï¼Œæ‰€ä»¥åŠ å…¥è¿™ä¸ªé›†åˆä»¥åŽï¼Œæ‰€ä»¥ä»¥å‰è¦†ç›–è¿™ä¸ªå˜é‡çš„é›†åˆscoreå–å€¼å¿…é¡»åŠ ä¸Šè¿™ä¸ªå˜é�?
		}
	}
}
void remove(int c)
{
	// printf("r%d\n",c);
	cs[c].is_in_c = 0;
	cs[c].score = -cs[c].score;
	cs[c].config = 0;
	// cs[c].config=1;
	int i, j, k, t, cnt, s, h, l;
	for (h = 0; h < nrow[c]; h++)
	{
		i = v_edges[c][h];
		cnt = 0;
		for (l = 0; l < 2; l++)
		{
			j = e_vertex[i][l];
			if (j == c)
				continue;
			if (cs[j].is_in_c)
			{
				cnt++;
				s = j;
			}
			cs[j].config = 1;
		}
		if (cnt == 0)
		{
			uncover[uncover_num] = i;
			index_uncover[i] = uncover_num;
			uncover_num++;
			for (l = 0; l < 2; l++)
			{
				j = e_vertex[i][l];
				if (j == c)
					continue;
				cs[j].score += weight[i];
			}
		}
		else if (cnt == 1)
		{
			cs[s].score -= weight[i];
		}
	}
}
int in_tabu(int i)
{
	return tabu_list.find(i) != tabu_list.end();
}
int find_best_in_c(int allowTabu)
{
	int i, maxc, j, k;
	int sr = INT_MIN, ct = 1;
	for (i = 0; i < v_num; i++)
	{
		if (!cs[i].is_in_c)
			continue;
		if (allowTabu && in_tabu(i))
			continue;
		if (cs[i].must_in)
			continue;
		k = compare(sr, ct, cs[i].score, cs[i].cost);
		if (sr == INT_MIN || k < 0)
		{
			sr = cs[i].score;
			ct = cs[i].cost;
			maxc = i;
		}
		else if (k == 0)
		{
			if (cs[maxc].time_stamp > cs[i].time_stamp)
			{
				maxc = i;
			}
		}
	}
	return maxc;
}
void uncov_r_weight_inc()
{
	int i, j, k, total = 0, cnt, s, ix, h, l;
	for (i = 0; i < uncover_num; i++)
	{
		weight[uncover[i]] += 1;

		cs[e_vertex[uncover[i]][0]].config = 1;
		cs[e_vertex[uncover[i]][1]].config = 1;

		for (h = 0; h < 2; h++)
		{
			j = e_vertex[uncover[i]][h];
			cs[j].score += 1;
		}
	}
}
void localsearch(int maxStep)
{
	step = 1;
	int i, j, k, h, l;
	int best_in_c;
	int maxc;
	int NoImpro = 0;
	int NumRemove = 3;
	while (step <= maxStep)
	{
		if (uncover_num == 0)
		{
			int k = cost_C();
			if (k < best_value)
			{
				update_best_sol();
				NoImpro = 0;
			}
			else
				NoImpro++;
			if (best_value == BEST)
			{
				printf("optimum found\n");
				break;
			}
			maxc = find_best_in_c(0);
			remove(maxc);
			continue;
		}
		if (NumRemove > 1 && NoImpro <= 100)
		{
			NumRemove--;
			NoImpro = 0;
		}

		for (int hh = 0; hh < NumRemove; hh++)
		{
			best_in_c = find_best_in_c(1);
			remove(best_in_c);
			cs[best_in_c].time_stamp = step;
		}
		/*if(step<=100000){
			for(int hh=0; hh<4;hh++)
			{
				best_in_c=find_best_in_c(1);
					remove(best_in_c);
				cs[best_in_c].time_stamp=step;
			}
		}
		else {
			for(int hh=0; hh<2;hh++)
			   {
				   best_in_c=find_best_in_c(1);
					   remove(best_in_c);
				   cs[best_in_c].time_stamp=step;
			   }
		}*/

		// best_in_c=find_best_in_c(1);
		tabu_list.clear();
		// remove(best_in_c);
		// cs[best_in_c].time_stamp=step;

		while (uncover_num > 0)
		{
			int cost = cost_C();
			if (cost >= best_value - 1)
				break;
			k = rand() % uncover_num;
			i = uncover[k];
			int sr = INT_MIN, ct;
			maxc = -1;
			for (h = 0; h < 2; h++)
			{
				j = e_vertex[i][h];
				// if(!cs[j].config) continue;

				if (cs[j].config == 0)
				{

					if (cs[j].cost + cost >= best_value)
						continue;
					int j_uncover_edges = 0;
					for (int ii = 0; ii < cs[j].degree; ii++)
					{
						int ee = v_edges[j][ii];
						if (index_uncover[ee] != -1)
							j_uncover_edges++;
					}
					if (j_uncover_edges == uncover_num)
					{
						maxc = j;
					}
				}
				else
				{
					k = compare(sr, ct, cs[j].score, cs[j].cost);
					if (sr == INT_MIN || k < 0)
					{
						sr = cs[j].score;
						ct = cs[j].cost;
						maxc = j;
					}
					else if (k == 0 && cs[maxc].time_stamp > cs[j].time_stamp)
					{
						maxc = j;
					}
				}
			}
			// if(cost+cs[maxc].cost>=best_value) break;
			add(maxc);
			tabu_list.insert(maxc);
			cs[maxc].time_stamp = step;
			uncov_r_weight_inc();
		}

		//		int step_tep;
		//		if(step%1000==0)
		//		{
		//			step_tep++;
		//			int num_tmp=0;
		//			for(int ll=0; ll<n; ll++){
		//				if(cs[ll].config==1)
		//					num_tmp++;
		//			}
		//			//printf("%10d     %10d\n",step_tep,num_tmp);
		//		}

		step++;
		if (step >= total_step)
			break;
		// if(cpu_time()-start_time>time_limit) break;
		times(&_end);
		double elap_time = (_end.tms_utime + _end.tms_stime - start_time) / sysconf(_SC_CLK_TCK);

		if (elap_time >= time_limit)
		{
			// printf("step%15ld",step);
			return;
		}
	}
	// printf("step%15ld",step);
	// printf("step==== %d\n",step);
}
int cost_C()
{
	int s = 0, j;
	for (j = 0; j < v_num; j++)
	{
		if (cs[j].is_in_c)
		{
			s += cs[j].cost;
		}
	}
	return s;
}
void update_best_sol()
{
	int i, j;
	int k = cost_C();
	if (k < best_value)
	{
		best_value = k;
		for (j = 0; j < v_num; j++)
		{
			best_sol[j] = 0;
			if (cs[j].is_in_c)
			{
				best_sol[j] = 1;
			}
		}
		// printf("sol %d, time %f, step %d\n", k, cpu_time()-start_time,step);
	}
	times(&_end);
	best_comp_time = double(_end.tms_utime - start.tms_utime + _end.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
	best_comp_time = round(best_comp_time * 100) / 100.0;
}
int check()
{ // check if the solution is a correct cover
	int i, j, k;
	static int t[MAXE];
	memset(t, 0, MAXE * sizeof(int)); // å°†å·²å¼€è¾Ÿå†…å­˜ç©ºé�?t çš„é¦�?MAXE ä¸ªå­—èŠ‚çš„å€¼è®¾ä¸ºå�?0
	for (j = 0; j < v_num; j++)
	{
		if (best_sol[j] == 0)
			continue;
		for (k = 0; k < nrow[j]; k++)
		{
			t[v_edges[j][k]] = 1;
		}
	}
	for (i = 0; i < e_num; i++)
	{
		if (!t[i])
			return 0;
	}
	return 1;
}
void print_sol()
{
	int j;
	for (j = 0; j < v_num; j++)
	{
		if (best_sol[j])
			printf("%d\t", j);
	}
	printf("\n");
}

bool ExeRule1()
{

	int i;
	bool isAdd = false;
	int v, numNotInC, n1, n2;
	int num = 0;
	int CycleNum = 0;
	while (CycleNum < 1)
	{

		if (uncover_num == 0)
			break;
		bool isFFind = false;
		struct tms st, ed;
		times(&st);
		for (v = 0; v < v_num; v++)
		{

			/*if(v%100==0){

				times(&_end);
						if((_end.tms_utime-start.tms_utime)*1.0/sysconf(_SC_CLK_TCK)>10){
					initTime=true;
				return false;
				}

			}*/
			// if(v==217) printf("v===%d\n",v);
			if (uncover_num == 0)
				break;
			if (cs[v].is_in_c == 1)
				continue;

			int *neighborNotInC = new int[cs[v].degree];
			numNotInC = 0;

			// printf("v===%d\n",v);

			if (cs[v].score == 1)
			{

				for (i = 0; i < cs[v].degree; i++)
				{
					int temp = v_adj[v][i];
					if (cs[temp].is_in_c == 0)
					{
						neighborNotInC[numNotInC++] = temp;
					}
				}
			}

			if (numNotInC == 1)
			{

				n1 = neighborNotInC[0];

				if (cs[v].cost >= cs[n1].cost)
				{
					isAdd = true;
					isFFind = true;
					add(n1);
					cs[n1].must_in = 1;
					num++;
					// printf("num== %d\n",num);
				}
			}
			delete[] neighborNotInC;
			times(&ed);

			// if(v%10000==0) printf("times=%.6f\n",(ed.tms_utime-st.tms_utime)*1.0/sysconf(_SC_CLK_TCK));
		}
		CycleNum++;
		if (!isFFind)
			break;
	}

	if (isAdd)
		return true;
	else
		return false;
}

bool ExeRule2()
{
	int v, numNotInC, n1, n2;
	int i;
	bool isAdd = false;
	int CycleNum = 0;
	while (CycleNum < 1)
	{
		if (uncover_num == 0)
			break;
		bool isFind = false;
		for (v = 0; v < v_num; v++)
		{
			/*if(v%100==0){

				times(&_end);
						if((_end.tms_utime-start.tms_utime)*1.0/sysconf(_SC_CLK_TCK)>10){
					initTime=true;
				return false;
				}

			}*/
			if (uncover_num == 0)
				break;
			if (cs[v].is_in_c == 1)
				continue;
			int *neighborNotInC = new int[cs[v].degree];
			numNotInC = 0;
			if (cs[v].score == 2)
			{

				for (i = 0; i < cs[v].degree; i++)
				{
					int temp = v_adj[v][i];
					if (cs[temp].is_in_c == 0)
					{
						neighborNotInC[numNotInC++] = temp;
					}
				}
			}

			if (numNotInC == 2)
			{
				n1 = neighborNotInC[0];
				n2 = neighborNotInC[1];
				bool is = false;
				for (i = 0; i < cs[n1].degree; i++)
				{
					if (v_adj[n1][i] == n2)
					{
						is = true;
						break;
					}
				}

				if (is)
				{
					if (cs[v].cost > cs[n1].cost + cs[n2].cost)
					{
						isFind = true;
						isAdd = true;
						cs[n1].must_in = 1;
						cs[n2].must_in = 1;
						add(n1);
						add(n2);
					}
				}
			}
			delete[] neighborNotInC;
		}
		if (!isFind)
			break;
		CycleNum++;
	}

	if (isAdd)
		return true;
	else
		return false;
}

bool ExeRule3()
{
	int v, numNotInC, n1, n2;
	int i;
	bool isAdd = false;
	int CycleNum = 0;
	while (CycleNum < 1)
	{
		if (uncover_num == 0)
			break;
		bool isFind = false;
		for (v = 0; v < v_num; v++)
		{
			/*if(v%100==0){

				times(&_end);
						if((_end.tms_utime-start.tms_utime)*1.0/sysconf(_SC_CLK_TCK)>10){
					initTime=true;
				return false;
				}

			}*/

			if (uncover_num == 0)
				break;
			if (cs[v].is_in_c == 1)
				continue;

			int *neighborNotInC = new int[cs[v].degree];
			numNotInC = 0;
			if (cs[v].score == 2)
			{

				for (i = 0; i < cs[v].degree; i++)
				{
					int temp = v_adj[v][i];
					if (cs[temp].is_in_c == 0)
					{
						neighborNotInC[numNotInC++] = temp;
					}
				}
			}

			if (numNotInC == 2)
			{
				n1 = neighborNotInC[0];
				n2 = neighborNotInC[1];
				bool is = false;
				for (i = 0; i < cs[n1].degree; i++)
				{
					if (v_adj[n1][i] == n2)
					{
						is = true;
						break;
					}
				}

				if (is)
				{

					if (cs[n1].score == 2)
					{
						isFind = true;
						isAdd = true;
						if (cs[v].cost < cs[n1].cost)
						{
							cs[v].must_in = 1;
							add(v);
						}
						else
						{
							cs[n1].must_in = 1;
							add(n1);
						}
					}
					else if (cs[n2].score == 2)
					{
						isFind = true;
						isAdd = true;
						if (cs[v].cost < cs[n2].cost)
						{
							cs[v].must_in = 1;
							add(v);
						}
						else
						{
							cs[n2].must_in = 1;
							add(n2);
						}
					}
				}
			}
			delete[] neighborNotInC;
		}
		if (!isFind)
			break;
		CycleNum++;
	}

	if (isAdd)
		return true;
	else
		return false;
}

bool ExeRule4()
{
	int v, numNotInC, n1, n2;
	int i;
	bool isAdd = false;
	int CycleNum = 0;
	while (CycleNum < 1)
	{
		if (uncover_num == 0)
			break;
		bool isFFind = false;
		for (v = 0; v < v_num; v++)
		{

			/*if(v%100==0){

				times(&_end);
						if((_end.tms_utime-start.tms_utime)*1.0/sysconf(_SC_CLK_TCK)>10){
					initTime=true;
				return false;
				}

			}*/
			if (uncover_num == 0)
				break;
			if (cs[v].is_in_c == 1)
				continue;

			int *neighborNotInC = new int[cs[v].degree];
			numNotInC = 0;
			if (cs[v].score == 2)
			{

				for (i = 0; i < cs[v].degree; i++)
				{
					int temp = v_adj[v][i];
					if (cs[temp].is_in_c == 0)
					{
						neighborNotInC[numNotInC++] = temp;
					}
				}
			}

			if (numNotInC == 2)
			{
				n1 = neighborNotInC[0];
				n2 = neighborNotInC[1];
				bool is = false;
				for (i = 0; i < cs[n1].degree; i++)
				{
					if (v_adj[n1][i] == n2)
					{
						is = true;
						break;
					}
				}

				if (!is)
				{
					bool isfind = false;
					int u;
					for (i = 0; i < cs[n1].degree; i++)
					{
						u = v_adj[n1][i];
						if (u == v)
							continue;
						if (cs[u].is_in_c == 0 && cs[u].score == 2)
						{
							for (int ii = 0; ii < cs[n2].degree; ii++)
							{
								if (v_adj[n2][ii] == u)
								{
									isfind = true;
								}
							}
							if (isfind)
								break;
						}
					}
					if (isfind)
					{
						if (cs[v].cost + cs[u].cost > cs[n1].cost + cs[n2].cost)
						{

							isAdd = true;
							isFFind = true;
							add(n1);
							add(n2);
							cs[n1].must_in = true;
							cs[n2].must_in = true;
						}
					}
				}
			}
			delete[] neighborNotInC;
		}
		if (!isFFind)
			break;
		CycleNum++;
	}

	if (isAdd)
		return true;
	else
		return false;
}

void Reduction()
{

	int i, v, e;
	bool initTime = false;
	// printf("%d\n",v_num);
	for (v = 0; v < v_num; v++)
	{
		cs[v].score = cs[v].degree;
		// printf("%d ",cs[v].score);
	}
	// printf("**************\n");
	// printf("76degree:%d %d %d \n",cs[75].score,cs[75].cost,cs[27].cost);
	// init();
	int num = 0;
	while (1)
	{
		if (uncover_num == 0)
			break;

		bool rule3 = ExeRule3(); // printf("---------------\n");

		bool rule4 = ExeRule4(); // printf("---------------\n");

		bool rule2 = ExeRule2(); // printf("---------------\n");

		bool rule1 = ExeRule1(); // printf("---------------\n");

		if (rule1 || rule2 || rule3 || rule4)
			continue;
		else
			break;
		num++;
		// printf("num==%d\n",num);
	}

	int numInC = 0;
	for (v = 0; v < v_num; v++)
	{

		cs[v].score = 0;
		if (cs[v].must_in == 1)
			numInC++;
	}
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("usage ./NuMWVC [Graph file] [Seed] [Cutoff time]\n");
		return 0;
	}

	// printf("%s",argv[1]);
	build_instance2(argv[1]);

	// sscanf(argv[2],"%d",&seed);
	// sscanf(argv[3],"%d",&time_limit);

	BEST = 0;
	// seed=1;
	// time_limit= 1000;

	// BEST=atoi(argv[2]);
	seed = atoi(argv[2]);
	time_limit = atof(argv[3]);
	// printf("%d %d %f\n",BEST,seed, time_limit);

	// total_step=INT_MAX;
	// total_step=1000000;
	total_step = 1000000;
	srand(seed);
	init();

	times(&start);

	// printf("v_num %d e_num %d \n",v_num,e_num);

	Reduction();
	init_fast();

	times(&_end);
	printf("%s,", argv[1]);

	// printf("%50s \n",argv[1]);
	// printf("init value:%15d ",best_value);
	// printf("init time: %10.6f \n",(_end.tms_utime-start.tms_utime)*1.0/sysconf(_SC_CLK_TCK));

	localsearch(total_step);

	times(&_end);

	printf("%d,%f\n", best_value, best_comp_time);

	// printf("\n final best: %15d ",best_value);
	// printf("search steps is %d, seed is %d\n",step,seed);
	if (!check())
		printf(",wrong answer");
	// printf("time cost %f s\n", (_end.tms_utime-start.tms_utime)*1.0/sysconf(_SC_CLK_TCK));
	// printf("time cost %15f s\n", best_comp_time);
	// print_sol();
	free_all();
	return 0;
}
