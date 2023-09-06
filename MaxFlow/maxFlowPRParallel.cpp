#include <bits/stdc++.h>
#include <omp.h>
#include <sys/time.h>
#include <atomic>

using namespace std;

struct Index
{
    int val = -1;
};

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
    bool *active, *discovered;

    Graph(int ve)
    {
        v = ve;
        adj_list.resize(v);
        excess_flow.resize(v);
        height.resize(v);
        new_height.resize(v);
        added_excess.resize(v);
        active = new bool[v];
        discovered = new bool[v];
        memset(active, false, sizeof *active * v);
        memset(discovered, false, sizeof *discovered * v);
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

void globalRelabel(Graph &g, int s, int t)
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

                    if (g.adj_list[g.adj_list[i][j].dest][g.adj_list[i][j].rev_index].capacity - g.adj_list[g.adj_list[i][j].dest][g.adj_list[i][j].rev_index].flow > 0 && g.height[g.adj_list[i][j].dest] == g.v && g.adj_list[i][j].dest != s)
                    {
                        int doNotProceed = 0;
#pragma omp atomic capture
                        doNotProceed = discovered[g.adj_list[i][j].dest]++;
                        if (doNotProceed == 0)
                        {
                            g.height[g.adj_list[i][j].dest] = g.height[i] + 1;
                            // #pragma omp critical
                            //                             {
                            //                                 cout<<g.adj_list[g.adj_list[i][j].dest][g.adj_list[i][j].rev_index].dest<<endl;
                            //                                 cout <<i<<" <- "<< g.adj_list[i][j].dest << endl;
                            //                             }
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
    // g.height[s] = g.v;
    // for(int i=0;i<g.v;i++){
    //     cout<<i << ": "<<g.height[i]<<" ,";
    // }
    // cout<<endl;
}

void push(Graph &g, int v, int t)
{
    for (int j = 0; j < g.adj_list[v].size(); j++)
    {
        if (g.height[v] == g.height[g.adj_list[v][j].dest] + 1 && g.adj_list[v][j].capacity - g.adj_list[v][j].flow > 0)
        {
            int flow = min(g.adj_list[v][j].capacity - g.adj_list[v][j].flow, g.excess_flow[v]);
            g.excess_flow[v] -= flow;
#pragma omp atomic
            g.added_excess[g.adj_list[v][j].dest] += flow;
            g.adj_list[v][j].flow += flow;
            g.adj_list[g.adj_list[v][j].dest][g.adj_list[v][j].rev_index].flow -= flow;
            if (g.adj_list[v][j].dest != t)
            {
                g.discovered[g.adj_list[v][j].dest] = true;
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
            globalRelabel(g, s, t);
        }
        count++;
        // for (int i = 0; i < g.v; i++)
        // {
        //     cout << i << ":" << g.added_excess[i] << "," << g.active[i] << "," << g.discovered[i] << "," << g.excess_flow[i] << "," << g.height[i] << "   ";
        // }
        // cout << endl;
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (g.active[i])
            {
                push(g, i, t);
            }
        }
        // for (int i = 0; i < g.v; i++)
        // {
        //     cout << i << ":" << g.added_excess[i] << "," << g.active[i] << "," << g.discovered[i] << "," << g.excess_flow[i] << "," << g.height[i] << "   ";
        // }
        // cout << endl;

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
#pragma omp parallel for schedule(static)
        for (int i = 0; i < g.v; i++)
        {
            if (g.height[i] < g.v && g.discovered[i])
            {
                g.active[i] = true;
                g.active_cnt++;
            }
            else
            {
                g.active[i] = false;
            }
            g.discovered[i] = false;
        }
        // for (int i = 0; i < g.v; i++)
        // {
        //     cout << i << ":" << g.added_excess[i] << "," << g.active[i] << "," << g.discovered[i] << "," << g.excess_flow[i] << "," << g.height[i] << "   ";
        // }
        // cout << endl;
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
    }

    return g.added_excess[t] + g.excess_flow[t];
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
    smallTestCase1();
    smallTestCase2();
    smallTestCase3();
    smallTestCase4();
}

int setThreads(Graph &g, int &s, int &t, int num)
{
    Graph *gt = new Graph(g.v);
    *gt = g;
    omp_set_num_threads(num);
    return maxflow(*gt, s, t);
    free(gt);
}

int main(int argc, char *argv[])
{
    // prechecks(8);
    int v = atoi(argv[2]);
    int s = atoi(argv[3]);
    int t = atoi(argv[4]);
    string str;
    cout << argv[1] << endl;
    ifstream graphfile(argv[1]);
    Graph g(v);
    int num;
    int i, j, ind = 0;
    map<int, Index> idToIndex;
    map<int, Index>::iterator it;
    while (getline(graphfile, str))
    {
        stringstream ss(str);
        string word;
        int source, destination, weight;
        getline(ss, word, ',');
        source = stoi(word);
        getline(ss, word, ',');
        destination = stoi(word);
        getline(ss, word, ',');
        weight = abs(stoi(word));
        if (idToIndex[source].val == -1)
        {
            idToIndex[source].val = ind;
            ind++;
        }
        if (idToIndex[destination].val == -1)
        {
            idToIndex[destination].val = ind;
            ind++;
        }
        g.addEdge(idToIndex[source].val, idToIndex[destination].val, weight);
    }
    graphfile.close();
    cout << g.e << endl;
    struct timeval start, end;
    double time_taken = 0;
    int res = setThreads(g, s, t, 1);
    cout << res << endl;
    for (num = 1; num <= 20; num = num * 2)
    {
        Graph *gt = new Graph(v);
        *gt = g;
        omp_set_num_threads(num);
        gettimeofday(&start, NULL);
        int tres = maxflow(*gt, s, t);
        gettimeofday(&end, NULL);
        if (res == tres)
        {
            cout << "Success!" << endl;
        }
        else
        {
            cout << "Error! Incorrect results" << endl;
            cout << tres << endl;
        }
        time_taken = (end.tv_sec - start.tv_sec) * 1e6;
        time_taken = (time_taken + (end.tv_usec -
                                    start.tv_usec)) *
                     1e-6;
        cout << "Threads: " << num << " Time: " << fixed
             << time_taken << setprecision(10) << endl;
        free(gt);
        if (num == 16)
            num = 10;
    }
    return 0;
}