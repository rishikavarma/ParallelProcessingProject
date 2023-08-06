#include <bits/stdc++.h>
#include <omp.h>
#include <sys/time.h>
#include <atomic>

using namespace std;

int gs, gt;

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

    void set_rev_index(int ri)
    {
        rev_index = ri;
    }
};

struct Graph
{
    int v = 0, e = 0;
    vector<int> excess_flow;
    vector<int> height;
    vector<vector<Edge>> adj_list;
    Graph(int ve)
    {
        v = ve;
        adj_list.resize(v);
        excess_flow.resize(v);
        height.resize(v);
    }

    void addEdge(int s, int d, int c)
    {
        for(int i=0; i<adj_list[s].size();i++){
            if(adj_list[s][i].dest==d){
                adj_list[s][i].capacity+=c;
                return;
            }
        }
        Edge te(d, c), be(s, 0);
        be.set_rev_index(adj_list[s].size());
        te.set_rev_index(adj_list[d].size());
        adj_list[s].push_back(te);
        adj_list[d].push_back(be);
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
};
void preflow(Graph &g, int s)
{
    int i;
    g.height[s] = g.v;
#pragma omp parallel for default(shared) private(i)
    for (i = 0; i < g.adj_list[s].size(); i++)
    {
        g.adj_list[s][i].flow = g.adj_list[s][i].capacity;
        g.excess_flow[g.adj_list[s][i].dest] += g.adj_list[s][i].flow;
        g.adj_list[g.adj_list[s][i].dest][g.adj_list[s][i].rev_index].flow -= g.adj_list[s][i].flow;
    }
}

bool push(Graph &g, int u)
{
    bool pushed = false;
    int i, flow, currEflow;
#pragma omp parallel for default(shared) private(i, flow, currEflow)
    for (i = 0; i < g.adj_list[u].size(); i++)
    {
        if (g.adj_list[u][i].flow == g.adj_list[u][i].capacity)
            continue;
        if (g.height[u] > g.height[g.adj_list[u][i].dest])
        {

#pragma omp critical
            {

                currEflow = g.excess_flow[u];
                flow = min(currEflow, g.adj_list[u][i].capacity - g.adj_list[u][i].flow);
                if (flow > 0)
                    g.excess_flow[u] -= flow;
            }
            // cout << u << " " << g.adj_list[u][i].dest << " " << g.excess_flow[u] << " " << g.adj_list[u][i].capacity << " " << g.adj_list[u][i].flow << " " << flow << endl;
            if (flow > 0)
            {
                // #pragma omp atomic
                g.excess_flow[g.adj_list[u][i].dest] += flow;
                // #pragma omp atomic
                g.adj_list[u][i].flow += flow;
                // #pragma omp atomic
                g.adj_list[g.adj_list[u][i].dest][g.adj_list[u][i].rev_index].flow -= flow;
            }
            pushed = true;
        }
    }
    return pushed;
}

void relabel(Graph &g, int u)
{
    int min_h = INT_MAX;
    int i;
#pragma omp parallel for default(shared) private(i) reduction(min : min_h)
    for (i = 0; i < g.adj_list[u].size(); i++)
    {
        if (g.adj_list[u][i].flow == g.adj_list[u][i].capacity)
            continue;
        min_h = min(g.height[g.adj_list[u][i].dest], min_h);
    }
    g.height[u] = min_h + 1;
    // cout << "relabel: " << u << " " << min_h + 1 << endl;
}

int overFlowVertex(Graph &g)
{
    int res = -1, i;
#pragma omp parallel for default(shared) private(i)
    for (i = 1; i < g.v - 1; i++)
        if (g.excess_flow[i] > 0)
        {
            res = i;
        }
    return res;
}

int getMaxFlow(Graph &g, int s, int t)
{
    gs = s;
    gt = t;
    preflow(g, s);
    int ver = overFlowVertex(g);
    while (ver != -1)
    {
        {
            if (!push(g, ver))
            {
                relabel(g, ver);
            }
        }
        ver = overFlowVertex(g);
    }
    return g.excess_flow[t];
}

void smallTestCase1()
{
    int v = 6;
    Graph g(v);
    g.addEdge(0, 1, 16);
    g.addEdge(0, 2, 13);
    g.addEdge(1, 2, 10);
    g.addEdge(2, 1, 4);
    g.addEdge(1, 3, 12);
    g.addEdge(2, 4, 14);
    g.addEdge(3, 2, 9);
    g.addEdge(3, 5, 20);
    g.addEdge(4, 3, 7);
    g.addEdge(4, 5, 4);
    if (getMaxFlow(g, 0, 5) == 23)
    {
        // cout << "Test case 1 passes!" << endl;
    }
    else
    {
        cout << "Test case 1 fails:(" << endl;
    }
}

void smallTestCase2()
{
    int v = 6;
    Graph g(v);
    g.addEdge(0, 1, 13);
    g.addEdge(0, 2, 10);
    g.addEdge(2, 1, 3);
    g.addEdge(1, 3, 6);
    g.addEdge(1, 5, 7);
    g.addEdge(3, 4, 10);
    g.addEdge(4, 5, 5);
    if (getMaxFlow(g, 0, 5) == 12)
    {
        // cout << "Test case 2 passes!" << endl;
    }
    else
    {
        cout << "Test case 2 fails:(" << endl;
    }
}

void smallTestCase3()
{
    int v = 6;
    Graph g(v);
    g.addEdge(0, 1, 10);
    g.addEdge(0, 3, 10);
    g.addEdge(1, 3, 2);
    g.addEdge(1, 2, 4);
    g.addEdge(1, 4, 8);
    g.addEdge(3, 4, 9);
    g.addEdge(2, 5, 10);
    g.addEdge(4, 5, 10);
    g.addEdge(4, 2, 6);
    int res = getMaxFlow(g, 0, 5);
    if (res == 19)
    {
        // cout << "Test case 3 passes!" << endl;
    }
    else
    {
        cout << "Test case 3 fails:( :" << res << endl;
    }
}

void smallTestCase4()
{
    int v = 6;
    Graph g(v);
    g.addEdge(0, 1, 15);
    g.addEdge(0, 3, 4);
    g.addEdge(1, 2, 12);
    g.addEdge(2, 3, 3);
    g.addEdge(4, 1, 5);
    g.addEdge(3, 4, 10);
    g.addEdge(2, 5, 7);
    g.addEdge(4, 5, 10);

    if (getMaxFlow(g, 0, 5) == 14)
    {
        // cout << "Test case 4 passes!" << endl;
    }
    else
    {
        cout << "Test case 4 fails:(" << endl;
    }
}

void prechecks(int num)
{
    omp_set_num_threads(num);
    smallTestCase1();
    smallTestCase2();
    smallTestCase3();
    smallTestCase4();
}

int main()
{
    prechecks(8);
    return 0;
}