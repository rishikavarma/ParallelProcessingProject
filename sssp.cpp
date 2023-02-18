#include <omp.h>
#include <bits/stdc++.h>

using namespace std;

struct Edge{
	int src, dest, weight;
	Edge(int s, int d, int w){
		src = s;
		dest = d;
		weight = w;
	}
};

struct Graph{
	int v=0,e=0;
	vector<Edge> edges;
	Graph( int ve){
		v = ve;
	}
	
	void addEdge( int s, int d, int w){
		Edge te(s, d, w);
		edges.push_back(te);
		e++;
	}
	void printDist( int dist[]){
		for(int i=0; i<v; i++){
			cout<<i<<": "<<dist[i]<<endl;
		}
	}
	void sssp(int s){
		int i,j;
		int dist[v];
		#pragma omp parallel for default(shared) private(i) 
		for(i=0;i<v;i++){
			dist[i]=INT_MAX;
		}
		dist[s]=0;
		for(i=1; i<v; i++){
			int odist;
			bool change = false;
			#pragma omp parallel for default(shared) private(j, odist)  
			for(j=0; j<e; j++){
				int u1 = edges[j].src, u2 = edges[j].dest, w = edges[j].weight;
				if(dist[u1]!=INT_MAX){
					odist = dist[u2];
					#pragma omp critical
					dist[u2]=min(dist[u1]+w,dist[u2]);
					if(odist!=dist[u2]){
						change = true;
					}
				}
			}
			if(!change)break;
		}
		printDist(dist);
	}
};

int main(){
	/*int v = 5;
	Graph g(v);
	int source = 0;
	g.addEdge(0,1,-1);
	g.addEdge(0,2,4);
	g.addEdge(1,2,3);
	g.addEdge(1,3,2);
	g.addEdge(1,4,2);
	g.addEdge(3,2,5);
	g.addEdge(3,1,1);
	g.addEdge(4,3,-3);
	g.sssp(source);*/	
	int v = 30;
	Graph g(v);
	int source = 0;
	int e = rand()%435, u1, u2, w,i;
	int num;
	#pragma omp parallel for default(shared) private(i, u1,u2,w) 
	for( i=0; i< e; i++){
		u1 = rand()%v;
		u2 = rand()%v;
		while (u1==u2){
			u2 = rand()%v;
		}
		w = (rand()%40)-20;
		g.addEdge(u1,u2,w);
	}
	cout<<"Number of threads?"<<endl;
	cin>>num;
	omp_set_num_threads(num);
	clock_t start, end;
	start = clock();
	g.sssp(source);
	end=clock();
	double time_taken = double(end-start);
	cout<<"Time: "<<time_taken<<endl;
	
	

	return 0;
}
