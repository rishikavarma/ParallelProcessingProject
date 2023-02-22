#include <omp.h>
#include <bits/stdc++.h>
#include <sys/time.h>

using namespace std;

struct Edge{
	int src, dest, weight;
	Edge(int s, int d, int w){
		src = s;
		dest = d;
		weight = w;
	}
};

struct DistVal{
	int val = INT_MAX;
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
};

int main(int argc, char* argv[]){
	/*int v = 5;
	Graph g(v);
	int s = 0;
	g.sssp(source);*/	
	int v=atoi(argv[2]);
	string str;
	cout<<argv[1]<<endl;
	ifstream graphfile(argv[1]);
	Graph g(v);
	int s = atoi(argv[3]);
	int num;
	map<int,DistVal> dist;
	int i,j,odist;
	bool change = false;
	while (getline (graphfile, str)){
		stringstream ss(str);
		string word;
		int source, destination, weight;
		getline(ss, word, ',');
		source = stoi(word);
		getline(ss, word, ',');
		destination = stoi(word);
		getline(ss, word, ',');
		weight = stoi(word);	
		g.addEdge(source,destination,weight);
	}
	graphfile.close();
	cout<<g.e<<endl;
	struct timeval start, end;
	double time_taken=0;
	int u1,u2,w,tu1,tu2;
	for(num=1; num<=20; num++){
		omp_set_num_threads(num);	
	 	gettimeofday(&start, NULL);
		// #pragma omp parallel for default(shared) private(i) 
		// for(i=0;i<v;i++){
		// 	dist[i]=INT_MAX;
		// }
		dist[s].val=0;
		for(i=1; i<v; i++){
			change = false;
			#pragma omp parallel for default(shared) private(j, odist, tu1,tu2,u1,u2,w)  
			for(j=0; j<g.e; j++){
				u1 = g.edges[j].src;
				u2 = g.edges[j].dest;
				w = g.edges[j].weight;
				if(dist[u1].val!=INT_MAX){
					odist = dist[u2].val;
					#pragma omp critical
					{
						tu1=dist[u1].val;
						tu2=dist[u2].val;
						if(tu1+w<tu2){
							dist[u2].val=tu1+w;
							// cout<<tu1<<" "<<tu2<<endl;
						}
					}
					if(odist!=dist[u2].val){
						change = true;
					}
				}
			}
			if(!change)break;
		}
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - 
								start.tv_usec)) * 1e-6;
		// for(int i=0; i<v; i++){
		// 	cout<<i<<": "<<dist[i]<<endl;
		// }
	    cout<<"Threads: "<<num<<" Time: "<<fixed
         << time_taken << setprecision(10)<<endl;
	}
	

	return 0;
}
