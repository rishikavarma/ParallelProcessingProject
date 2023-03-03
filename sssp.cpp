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
	ofstream expected_output("expected_output.txt"),output1("output1.txt");
	Graph g(v);
	int s = atoi(argv[3]);
	int num;
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
	map<int,DistVal> dist,dist_test;
	map<int, DistVal>::iterator it;

	// sequential run for testing accuracy
	dist_test[s].val=0;
	for(i=1; i<v; i++){
		change = false;
		for(j=0; j<g.e; j++){
			u1 = g.edges[j].src;
			u2 = g.edges[j].dest;
			w = g.edges[j].weight;
			if(dist_test[u1].val!=INT_MAX){
				odist = dist_test[u2].val;
				dist_test[u2].val = min(dist_test[u2].val,dist_test[u1].val+w);
				if(odist!=dist_test[u2].val){
					change = true;
				}
			}
		}
		if(!change)break;
	}
	for(it = dist_test.begin(); it!=dist_test.end(); it++){
		expected_output<<it->first<<": "<<it->second.val<<endl;
	}
	cout<<"Sequential run end"<<endl;
	
	// Parallel runs for Number of threads: 1,2,4,8,16,20
	for(num=1; num<=20; num=num*2){
		omp_set_num_threads(num);	
		#pragma omp parallel default(shared) private(j, odist, tu1,tu2,u1,u2,w)  
		{
			gettimeofday(&start, NULL);
			dist[s].val=0;
			for(i=1; i<v; i++){
				change = false;
				#pragma omp for 
				for(j=0; j<g.e; j++){
					u1 = g.edges[j].src;
					u2 = g.edges[j].dest;
					w = g.edges[j].weight;
					if(dist[u1].val!=INT_MAX){
						odist = dist[u2].val;
						#pragma omp critical
						{
							cout<<u1<<" "<<u2<<" thread num: "<<omp_get_thread_num()<<" "<<num;
							tu1=dist[u1].val;
							tu2=dist[u2].val;
							if(tu1+w<tu2){
								dist[u2].val=tu1+w;
								cout<<" here";
							}
							cout<<" done";
							// dist[u2].val = min(dist[u2].val,dist[u1].val+w);
						}
						if(odist!=dist[u2].val){
							change = true;
						}
						cout<<"next_iter"<<endl;
					}
					cout<<"out0 "<<change<<" "<<j<<" "<<g.e<<endl;
				}
				cout<<"out01 "<<j<<" "<<g.e<<endl;
				if(!change)break;
			}
			cout<<"out"<<endl;
			gettimeofday(&end, NULL);
		}
		cout<<"out2"<<endl;
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - 
								start.tv_usec)) * 1e-6;
		for(it = dist.begin(); it!=dist.end(); it++){
			output1<<it->first<<": "<<it->second.val<<endl;
			cout<<it->first<<": "<<it->second.val<<endl;
		}						
		for(it = dist.begin(); it!=dist.end(); it++){
			if(dist_test[it->first].val!=it->second.val){
				cout<<"Error! Incorrect results"<<endl;
				break;
			}
			it->second.val=INT_MAX;
		}
		if(it!=dist.end())break;
	    cout<<"Threads: "<<num<<" Time: "<<fixed
         << time_taken << setprecision(10)<<" Iterations: "<<i<<endl;
		 if(num==16)num=10;
	}
	return 0;
}
