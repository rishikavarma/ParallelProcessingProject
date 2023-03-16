#include <omp.h>
#include <bits/stdc++.h>
#include <sys/time.h>

using namespace std;

struct Edge
{
	int src, dest, weight;
	Edge(int s, int d, int w)
	{
		src = s;
		dest = d;
		weight = w;
	}
};

struct Index
{
	int val = -1;
};

struct Graph
{
	int v = 0, e = 0;
	vector<Edge> edges;
	Graph(int ve)
	{
		v = ve;
	}

	void addEdge(int s, int d, int w)
	{
		Edge te(s, d, w);
		edges.push_back(te);
		e++;
	}
	void printDist(int dist[])
	{
		for (int i = 0; i < v; i++)
		{
			cout << i << ": " << dist[i] << endl;
		}
	}
};

int main(/*int argc, char* argv[]*/)
{
	int v = 5 /*atoi(argv[2])*/;
	string str;
	// cout<<argv[1]<<endl;
	// ifstream graphfile(argv[1]);
	ofstream expected_output("expected_output_r.txt"), output1("output1_r.txt");
	Graph g(v);
	int s = 0 /*atoi(argv[3])*/;
	int num;
	int i, j, k, odist;
	map<int, Index> idToIndex;
	int ind = 0;
	map<int, Index>::iterator it;
	bool change = false, initial = true;
	idToIndex[0].val = 0;
	idToIndex[1].val = 1;
	idToIndex[2].val = 2;
	idToIndex[3].val = 3;
	idToIndex[4].val = 4;
	g.addEdge(0, 1, -1);
	g.addEdge(0, 2, 4);
	g.addEdge(1, 2, 3);
	g.addEdge(1, 3, 2);
	g.addEdge(1, 4, 2);
	g.addEdge(3, 2, 5);
	g.addEdge(3, 1, 1);
	g.addEdge(4, 3, -3);
	// while (getline (graphfile, str)){
	// 	stringstream ss(str);
	// 	string word;
	// 	int source, destination, weight;
	// 	getline(ss, word, ',');
	// 	source = stoi(word);
	// 	getline(ss, word, ',');
	// 	destination = stoi(word);
	// 	getline(ss, word, ',');
	// 	weight = stoi(word);
	// 	g.addEdge(source,destination,weight);
	//     if(idToIndex[source].val == -1){
	//         idToIndex[source].val = ind;
	//         ind++;
	//     }
	//     if(idToIndex[destination].val == -1){
	//         idToIndex[destination].val = ind;
	//         ind++;
	//     }

	// }
	// graphfile.close();
	cout << g.e << endl;
	cout << ind << endl;
	struct timeval start, end;
	double time_taken = 0;
	int u1, u2, w, tu1, tu2;
	int sssp[v];
	int sssp_priv[v];
	int sssp_test[v];
	int iter = 0;
#pragma omp parallel for default(shared) private(i)
	for (i = 0; i < v; i++)
	{
		sssp_test[i] = INT_MAX;
		sssp[i] = INT_MAX;
	}

	// sequential run for testing accuracy
	sssp_test[idToIndex[s].val] = 0;
	for (i = 1; i < v; i++)
	{
		change = false;
		for (j = 0; j < g.e; j++)
		{
			u1 = g.edges[j].src;
			u2 = g.edges[j].dest;
			w = g.edges[j].weight;
			if (sssp_test[idToIndex[u1].val] != INT_MAX)
			{
				odist = sssp_test[idToIndex[u2].val];
				sssp_test[idToIndex[u2].val] = min(sssp_test[idToIndex[u2].val], sssp_test[idToIndex[u1].val] + w);
				if (odist != sssp_test[idToIndex[u2].val])
				{
					change = true;
				}
			}
		}
		if (!change)
			break;
	}
	for (i = 0; i < v; i++)
	{
		expected_output << i << ": " << sssp_test[i] << endl;
	}
	cout << "Sequential run end" << endl;

	for (num = 1; num <= 20; num = num * 2)
	{
		omp_set_num_threads(num);
		gettimeofday(&start, NULL);
		sssp[idToIndex[s].val] = 0;
		cout << idToIndex[s].val << " " << sssp[idToIndex[s].val] << endl;
		for (i = 1; i < v; i++)
		{
			change = false;
			initial = true;
			for (k = 0; k < v; k++)
			{
				cout << sssp[k] << " ";
			}
			cout << endl;
#pragma omp parallel for default(shared) private(j, k, odist, u1, u2, w, initial) reduction(min \
																							: sssp_priv[:v])
			for (j = 0; j < g.e; j++)
			{
				    if(initial){
						for(k=0;k<v;k++){
							sssp_priv[k]=sssp[k];
						}
						initial = false;
					}
				if (sssp_priv[idToIndex[s].val] == INT_MAX)
					sssp_priv[idToIndex[s].val] = 0;
				u1 = g.edges[j].src;
				u2 = g.edges[j].dest;
				w = g.edges[j].weight;
				// cout << i << " " << v << " " << j << " " << g.e << endl;
				if (sssp_priv[idToIndex[u1].val] != INT_MAX)
				{
					sssp_priv[idToIndex[u2].val] = min(sssp_priv[idToIndex[u2].val], sssp_priv[idToIndex[u1].val] + w);
				}
			}
#pragma omp parallel for default(shared) private(k)
			for(k=0;k<v;k++){
				if(sssp[k]!=sssp_priv[k]){
					sssp[k]=sssp_priv[k];
					change=true;
				}
			}
			if (!change)
				break;
		}
		iter = i;
		gettimeofday(&end, NULL);
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec -
									start.tv_usec)) *
					 1e-6;
		for (i = 0; i < v; i++)
		{
			output1 << i << ": " << sssp[i] << endl;
		}
		bool error = false;
#pragma omp parallel for default(shared) private(i) reduction(| \
															  : error)
		for (i = 0; i < v; i++)
		{
			if (sssp_test[i] != sssp[i])
			{
				error = true;
			}
			sssp[i] = INT_MAX;
			if (i == idToIndex[s].val)
				sssp[i] = 0;
		}
		if (error)
		{
			cout << "Error! Incorrect results" << endl;
			// break;
		}
		cout << "Threads: " << num << " Time: " << fixed
			 << time_taken << setprecision(10) << " Iterations: " << iter << endl;
		if (num == 16)
			num = 10;
	}
	return 0;
}
