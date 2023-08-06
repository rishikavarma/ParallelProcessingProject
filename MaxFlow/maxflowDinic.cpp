#include <bits/stdc++.h>
#include <omp.h>
#include <sys/time.h>
#include <atomic>

using namespace std;

struct Edge
{
    int dest, capacity, flow, rev_index;
    Edge(int d, int w)
    {
        dest = d;
        capacity = w;
        flow = 0;
        rev_index = -1;
    }

    void set_rev_index( int ri){
        rev_index = ri;
    }
};

struct Graph
{
    int v = 0, e = 0;
    vector<vector<Edge>> adj_list;
    vector<int> level;
    Graph(int ve)
    {
        v = ve;
        adj_list.resize(v);
        level.resize(v);
    }

    void addEdge(int s, int d, int w)
    {
        Edge te(d, w);
        adj_list[s].push_back(te);
        e++;
    }

    void printGraph(bool seq)
    {
        for (int i = 0; i < v; i++)
        {
            if (adj_list[i].size())
            {
                cout << i << "  :  ";
                for (int j = 0; j < adj_list[i].size(); j++)
                {
                    cout << " " << adj_list[i][j].dest << ",";
                }
                cout << endl;
            }
        }
    }

    bool bfs(int s, int t)
    {
        for (int i = 0; i < v; i++)
            level[i] = -1;
        level[s] = 0;
        list<int> q;
        q.push_back(s);
        while (!q.empty())
        {
            int u = q.front();
            q.pop_front();
            // Parallellise here.
            for (int i = 0; i < adj_list[u].size(); i++)
            {
                if (level[adj_list[u][i].dest] == -1 && adj_list[u][i].flow < adj_list[u][i].capacity)
                {
                    level[adj_list[u][i].dest] = level[u] + 1;
                    // atomic
                    q.push_back(adj_list[u][i].dest);
                }
            }
        }
        return level[t] >= 0;
    }

    int sendFlow(int u, int flow, int t, int *start) {
        if(u==t)return flow;
        for(;start[u]<adj_list[u].size();start[u]++){
            Edge& e = adj_list[u][start[u]];

            if(level[e.dest]==level[u]+1 && e.flow<e.capacity){
                int cf = min(flow, e.capacity-e.flow);
                int tf = sendFlow(e.dest, cf, t, start);
                if(tf>0){
                    e.flow += tf;
                    adj_list[e.dest][e.rev_index].flow -= tf;
                    return tf;
                }
            }
        }
        return 0;
    }
};

int maxFlowDinic(Graph &g, int s, int t)
{
    if (s == t)
        return -1;
    int maxFlow = 0, flow = 0;
    int *start = new int[g.v + 1];
    while (g.bfs(s, t))
    {
        memset(start, 0, sizeof *start);
        flow = g.sendFlow(s, INT_MAX, t, start);
        while (flow)
        {
            maxFlow += flow;
            flow = g.sendFlow(s, INT_MAX, t, start);
        }
    }
}