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

    void set_rev_index(int ri)
    {
        rev_index = ri;
    }
};

struct Graph
{
    int v = 0, e = 0, active_cnt = 0;
    vector<int> excess_flow;
    vector<int> added_excess;
    vector<int> height;
    vector<int> new_height;
    vector<vector<Edge>> adj_list;
    vector<bool> active, discovered;

    Graph(int ve)
    {
        v = ve;
        adj_list.resize(v);
        excess_flow.resize(v);
        height.resize(v);
        new_height.resize(v);
        added_excess.resize(v);
        active.resize(v);
        discovered.resize(v);
    }

    void addEdge(int s, int d, int c)
    {
        for (int i = 0; i < adj_list[s].size(); i++)
        {
            if (adj_list[s][i].dest == d)
            {
                adj_list[s][i].capacity += c;
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
        g.active[g.adj_list[s][i].dest] = true;
    }
    g.active_cnt = g.adj_list[s].size();
}

void globalRelabel(Graph &g, int t)
{
#pragma omp parallel for schedule(static)
    for (int i = 0; i < g.v; i++)
    {
        g.height[i] = g.v;
    }
    g.height[t] = 0;
    int queue_size = 1, disc_count = 0;
    int active[g.v] = {0}, discovered[g.v] = {0};
    active[t] = true;
    while (queue_size)
    {
        // cout << queue_size << endl;
        // for (int i = 0; i < g.v; i++)
        // {
        //     if (active[i])
        //     {
        //         cout << i << " ";
        //     }
        // }
        // cout << endl;
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (active[i])
            {
                for (int j = 0; j < g.adj_list[i].size(); j++)
                {

                    if (g.adj_list[g.adj_list[i][j].dest][g.adj_list[i][j].rev_index].capacity > 0 && g.height[g.adj_list[i][j].dest] == g.v)
                    {
                        int doNotProceed = 0;
#pragma omp atomic capture
                        doNotProceed = discovered[g.adj_list[i][j].dest]++;
                        if (doNotProceed == 0)
                        {
                            g.height[g.adj_list[i][j].dest] = g.height[i] + 1;
                        }
                    }
                }
            }
        }
        queue_size = 0;
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            active[i] = discovered[i];
            discovered[i] = 0;
            if (active[i])
            {
#pragma omp atomic
                queue_size++;
            }
        }
    }
    cout << queue_size << endl;
        for (int i = 0; i < g.v; i++)
        {
            
                cout << i << ": "<< g.height[i]<<", ";
            
        }
        cout << endl;
}

void push(Graph &g, int v, int t)
{
    for (int j = 0; j < g.adj_list[v].size(); j++)
    {
        if (g.height[v] == g.height[g.adj_list[v][j].dest] + 1 && g.adj_list[v][j].capacity > 0)
        {
            int flow = min(g.adj_list[v][j].capacity - g.adj_list[v][j].flow, g.excess_flow[v]);
            g.excess_flow[v] -= flow;
#pragma omp atomic
            g.added_excess[g.adj_list[v][j].dest] += flow;
            g.adj_list[v][j].flow += flow;
            g.adj_list[g.adj_list[v][j].dest][g.adj_list[v][j].rev_index].flow -= flow;
            if (g.adj_list[v][j].dest != t)
            {
#pragma omp critical
                {
                    cout << g.adj_list[v][j].dest << endl;
                    g.discovered[g.adj_list[v][j].dest] = true;
                }
            }

            if (g.excess_flow[v] == 0)
                return;
        }
    }
}

int calculate_new_height(Graph &g, int v)
{
    int nh = g.v - 1;
    for (int i = 0; i < g.adj_list[v].size(); i++)
    {
        if (g.adj_list[v][i].capacity == g.adj_list[v][i].flow)
            continue;
        nh = min(nh, g.height[g.adj_list[v][i].dest]);
    }
    return nh + 1;
}

void relabel(Graph &g, int v)
{
    if (g.excess_flow[v] > 0 || g.height[v] == g.v)
    {
        g.new_height[v] = calculate_new_height(g, v);
        if (g.new_height[v] != g.v && g.new_height[v] != g.height[v])
            g.discovered[v] = true;
    }
    else
    {
        g.new_height[v] = g.height[v];
    }
}

int maxflow(Graph &g, int s, int t)
{
    int count = 0;
    preflow(g, s);

    while (g.active_cnt)
    {
        if (count % 4 == 0)
        {
            globalRelabel(g, t);
        }
        count++;
        // #pragma omp parallel
        // {
        for (int i = 0; i < g.v; i++)
        {
            cout << i << ":" << g.added_excess[i] << "," << g.active[i] << "," << g.discovered[i] << "," << g.excess_flow[i] << "    ";
        }
        cout << endl;
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (g.active[i])
            {
                push(g, i, t);
            }
        }
        for (int i = 0; i < g.v; i++)
        {
            cout << i << ":" << g.added_excess[i] << "," << g.active[i] << "," << g.discovered[i] << "," << g.excess_flow[i] << "    ";
        }
        cout << endl;

#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (g.active[i])
            {
                relabel(g, i);
            }
        }
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (g.active[i])
            {
                g.height[i] = g.new_height[i];
#pragma omp atomic
                g.excess_flow[i] += g.added_excess[i];
                g.added_excess[i] = 0;
            }
        }
        g.active_cnt = 0;
        for (int i = 0; i < g.v; i++)
        {
            if (g.height[i] < g.v && g.discovered[i])
            {
                
                cout<<i<<endl;
            }
            
        }
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (g.height[i] < g.v && g.discovered[i])
            {
                // #pragma omp critical
                // {
                //     cout<<i<<endl;
                // }
                g.active[i] = true;
#pragma omp atomic
                g.active_cnt++;
            }
            else
            {
                g.active[i] = false;
            }
            g.discovered[i] = false;
        }
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (g.active[i])
            {
#pragma omp atomic
                g.excess_flow[i] += g.added_excess[i];
                g.added_excess[i] = 0;
            }
        }
        // }
    }

    return g.excess_flow[s] + g.added_excess[t];
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
    int res = maxflow(g, 0, 5);
    if (res == 23)
    {
        cout << "Test case 1 passes!" << endl;
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
    int res = maxflow(g, 0, 5);
    if (res == 12)
    {
        cout << "Test case 2 passes!" << endl;
    }
    else
    {
        cout << "Test case 2 fails:( :" << res << endl;
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
    int res = maxflow(g, 0, 5);
    if (res == 19)
    {
        cout << "Test case 3 passes!" << endl;
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
    int res = maxflow(g, 0, 5);
    if (res == 14)
    {
        cout << "Test case 4 passes!" << endl;
    }
    else
    {
        cout << "Test case 4 fails:( :" << res << endl;
    }
}

void prechecks(int num)
{
    omp_set_num_threads(num);
    // smallTestCase1();
    // smallTestCase2();
    smallTestCase3();
    // smallTestCase4();
}

int main()
{
    prechecks(2);
    return 0;
}