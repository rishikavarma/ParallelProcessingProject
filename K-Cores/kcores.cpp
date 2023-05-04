#include <bits/stdc++.h>
#include <omp.h>
#include <sys/time.h>
#include <atomic>

using namespace std;

ofstream output1kc("output1_kc.txt"), output2kc("output2_kc.txt"), expected_output("expected_output_kc.txt");

struct Index
{
  int val = -1;
};

struct Graph
{
  int v = 0, e = 0;
  vector<vector<int>> adj_list;
  vector<int> degree;
  int min_degree = INT_MAX;
  int start_vertex;
  Graph(int ve)
  {
    v = ve;
    adj_list.resize(v);
    degree.resize(v);
  }

  void addDirectedEdge(int s, int d)
  {
    adj_list[s].push_back(d);
    degree[s]++;
  }

  void addEdge(int s, int d)
  {
    addDirectedEdge(s, d);
    addDirectedEdge(d, s);
    e++;
  }

  void printKCoresGraph(int k)
  {
    for (int i = 0; i < v; i++)
    {
      if (degree[i] >= k)
      {
        output2kc << i << "  :  ";
        for (int j = 0; j < adj_list[i].size(); j++)
        {
          if (degree[adj_list[i][j]] >= k)
          {
            output2kc << " " << adj_list[i][j] << ",";
          }
        }
        output2kc << endl;
      }
    }
  }

  void printGraph(bool seq)
  {
    for (int i = 0; i < v; i++)
    {
      if (adj_list[i].size())
      {
        if (seq)
          expected_output << i << "  :  ";
        else
          output2kc << i << "  :  ";
        for (int j = 0; j < adj_list[i].size(); j++)
        {
          if (seq)
            expected_output << " " << adj_list[i][j] << ",";
          else
            output2kc << " " << adj_list[i][j] << ",";
        }
        if (seq)
          expected_output << endl;
        else
          output2kc << endl;
      }
    }
  }
};

void compute_k_cores_dfs(Graph &g, int *&visited, int v, int k)
{
  int doNotProceed = 0;
#pragma omp atomic capture
  doNotProceed = visited[v]++;
  if (doNotProceed > 0)
    return;
  for (int i = 0; i < g.adj_list[v].size(); i++)
  {
    int adj_v = g.adj_list[v][i];
    g.degree[adj_v]--;
    if (!visited[adj_v] && g.degree[adj_v] < k)
    {
      // output1kc << adj_v << endl;
      compute_k_cores_dfs(g, visited, adj_v, k);
    }
  }
}

Graph *compute_k_cores(Graph &g, int k)
{
  int *visited = new int[g.v];
  int i;
#pragma omp parallel for default(shared) private(i)
  for (i = 0; i < g.v; i++)
  {
    if (!visited[i] && g.degree[i] < k)
    {
      // output1kc << i << endl;
      compute_k_cores_dfs(g, visited, i, k);
    }
  }
  Graph *kCore = new Graph(g.v);
  int j = 0;
#pragma omp parallel for default(shared) private(i, j)
  for (i = 0; i < g.v; i++)
  {
    if (g.degree[i] >= k)
    {
      for (j = 0; j < g.adj_list[i].size(); j++)
      {
        if (g.degree[g.adj_list[i][j]] >= k)
        {
          kCore->addDirectedEdge(i, g.adj_list[i][j]);
        }
      }
    }
  }
  kCore->printGraph(false);
  free(visited);
  return kCore;
}

void DFSUtil(Graph g, int v, int *&visited,
             int k)
{
  // Mark the current node as visited and print it
  visited[v] = true;

  // Recur for all the vertices adjacent to this vertex
  int i;
  for (i = 0; i != g.adj_list[v].size(); ++i)
  {
    // degree of v is less than k, then degree of adjacent
    // must be reduced
    if (g.degree[v] < k)
      g.degree[i]--;

    // If adjacent is not processed, process it
    if (!visited[i])
    {
      // If degree of adjacent after processing becomes
      // less than k, then reduce degree of v also.
      DFSUtil(g, i, visited, k);
    }
  }
}

Graph *printKCores(Graph &g, int k)
{
  // INITIALIZATION
  // Mark all the vertices as not visited and not
  // processed.
  int *visited = new int[g.v];
  int i, j, cnt;
  vector<bool> processed(g.v, false);

  int mindeg = INT_MAX;
  int startvertex;

  for (i = 0; i < g.v; i++)
  {
    g.degree[i] = g.adj_list[i].size();

    if (g.degree[i] < mindeg)
    {
      mindeg = g.degree[i];
      startvertex = i;
    }
  }
  DFSUtil(g, startvertex, visited, k);

  // If Graph is disconnected.
  for (i = 0; i < g.v; i++)
  {
    if (visited[i] == false)
      DFSUtil(g, i, visited, k);
  }
  // Considering Edge Case (Example 3 in main() function)
  for (i = 0; i < g.v; i++)
  {
    if (g.degree[i] >= k)
    {
      cnt = 0;
      for (j = 0; j < g.adj_list[i].size(); j++)
      {
        if (g.degree[g.adj_list[i][j]] >= k)
        {
          cnt++;
        }
      }
      if (cnt < k)
        g.degree[i] = cnt;
    }
  }
  // PRINTING K CORES
  Graph *kCore = new Graph(g.v);
  for (i = 0; i < g.v; i++)
  {
    if (g.degree[i] >= k)
    {
      for (j = 0; j < g.adj_list[i].size(); j++)
      {
        if (g.degree[g.adj_list[i][j]] >= k)
        {
          kCore->addDirectedEdge(i, g.adj_list[i][j]);
        }
      }
    }
  }
  kCore->printGraph(true);
  free(visited);
  return kCore;
}

void seq_compute_k_cores_dfs(Graph &g, int *&visited, int v, int k)
{

  visited[v] = 1;
  for (int i = 0; i < g.adj_list[v].size(); i++)
  {
    int adj_v = g.adj_list[v][i];
    g.degree[adj_v]--;
    if (!visited[adj_v] && g.degree[adj_v] < k)
    {
      // output1kc << adj_v << endl;
      compute_k_cores_dfs(g, visited, adj_v, k);
    }
  }
}

Graph *seq_KCores(Graph &g, int k)
{
  int *visited = new int[g.v];
  int i;
  for (i = 0; i < g.v; i++)
  {
    if (!visited[i] && g.degree[i] < k)
    {
      // output1kc << i << endl;
      seq_compute_k_cores_dfs(g, visited, i, k);
    }
  }
  Graph *kCore = new Graph(g.v);
  int j = 0;
  for (i = 0; i < g.v; i++)
  {
    if (g.degree[i] >= k)
    {
      for (j = 0; j < g.adj_list[i].size(); j++)
      {
        if (g.degree[g.adj_list[i][j]] >= k)
        {
          kCore->addDirectedEdge(i, g.adj_list[i][j]);
        }
      }
    }
  }
  kCore->printGraph(true);
  free(visited);
  return kCore;
}

void smallTestCase1()
{
  Graph g1(9);
  g1.addEdge(0, 1);
  g1.addEdge(0, 2);
  g1.addEdge(1, 2);
  g1.addEdge(1, 5);
  g1.addEdge(2, 3);
  g1.addEdge(2, 4);
  g1.addEdge(2, 5);
  g1.addEdge(2, 6);
  g1.addEdge(3, 4);
  g1.addEdge(3, 6);
  g1.addEdge(3, 7);
  g1.addEdge(4, 6);
  g1.addEdge(4, 7);
  g1.addEdge(5, 6);
  g1.addEdge(5, 8);
  g1.addEdge(6, 7);
  g1.addEdge(6, 8);
  output2kc << "G1:" << endl;
  // output1kc <<"G1:" << endl;
  Graph *res = printKCores(g1, 3);
  bool err = false;
  vector<int> ans{2, 3, 4, 6, 7};
  for (int i = 0; i < ans.size(); i++)
  {
    if (res->adj_list[ans[i]].size() == 0)
    {
      cout << "G1 error" << endl;
      err = true;
    }
  }
  if (!err)
    cout << "G1 successfull!" << endl;
  free(res);
}

void smallTestCase2()
{
  Graph g2(13);
  g2.addEdge(0, 1);
  g2.addEdge(0, 2);
  g2.addEdge(0, 3);
  g2.addEdge(1, 4);
  g2.addEdge(1, 5);
  g2.addEdge(1, 6);
  g2.addEdge(2, 7);
  g2.addEdge(2, 8);
  g2.addEdge(2, 9);
  g2.addEdge(3, 10);
  g2.addEdge(3, 11);
  g2.addEdge(3, 12);
  output2kc << "G2:" << endl;
  // output1kc << "G2:" << endl;
  Graph *res = printKCores(g2, 3);
  bool err = false;
  for (int i = 0; i < res->adj_list.size(); i++)
  {
    if (res->adj_list[i].size() != 0)
    {
      cout << "G2 error" << endl;
      err = true;
    }
  }
  if (!err)
    cout << "G2 successfull!" << endl;
  free(res);
}

void smallTestCase3()
{
  Graph gr(9);
  gr.addEdge(0, 1);
  gr.addEdge(0, 2);
  gr.addEdge(1, 2);
  gr.addEdge(2, 5);
  gr.addEdge(2, 4);
  gr.addEdge(2, 3);
  gr.addEdge(2, 6);
  gr.addEdge(3, 4);
  gr.addEdge(3, 6);
  gr.addEdge(3, 7);
  gr.addEdge(4, 6);
  gr.addEdge(5, 6);
  gr.addEdge(5, 7);
  gr.addEdge(5, 8);
  gr.addEdge(8, 7);
  output2kc << "G3:" << endl;
  // output1kc << "G3:" << endl;
  Graph *res = printKCores(gr, 3);
  bool err = false;
  vector<int> ans3{2, 3, 4, 6};
  for (int i = 0; i < ans3.size(); i++)
  {
    if (res->adj_list[ans3[i]].size() == 0)
    {
      cout << "G3 error" << endl;
      err = true;
    }
  }
  if (!err)
    cout << "G3 successfull!" << endl;
  free(res);
}

void smallTestCase4()
{
  Graph g4(10);
  g4.addEdge(0, 1);
  g4.addEdge(0, 4);
  g4.addEdge(0, 7);
  g4.addEdge(2, 1);
  g4.addEdge(4, 1);
  g4.addEdge(6, 1);
  g4.addEdge(2, 3);
  g4.addEdge(2, 4);
  g4.addEdge(3, 4);
  g4.addEdge(3, 5);
  g4.addEdge(3, 6);
  g4.addEdge(5, 4);
  g4.addEdge(5, 6);
  g4.addEdge(5, 7);
  g4.addEdge(5, 8);
  g4.addEdge(6, 9);
  g4.addEdge(7, 8);
  g4.addEdge(8, 9);
  output2kc << "G4:" << endl;
  // output1kc << "G4:" << endl;
  Graph *res = printKCores(g4, 3);
  vector<int> ans4{1, 2, 3, 4, 5, 6};
  bool err = false;
  for (int i = 0; i < ans4.size(); i++)
  {
    if (res->adj_list[ans4[i]].size() == 0)
    {
      cout << "G4 error" << endl;
      err = true;
    }
  }
  if (!err)
    cout << "G4 successfull!" << endl;
  free(res);
}

int main(int argc, char *argv[])
{
  int v = atoi(argv[2]);
  string str;
  cout << argv[1] << endl;
  ifstream graphfile(argv[1]);
  Graph g(v);
  int num;
  int i, j, ind = 0;
  map<int, Index> idToIndex;
  map<int, Index>::iterator it;
  Graph *res;
  while (getline(graphfile, str))
  {
    stringstream ss(str);
    string word;
    int source, destination;
    getline(ss, word, ',');
    source = stoi(word);
    getline(ss, word, ',');
    destination = stoi(word);
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
    for (i = 0; i < g.adj_list[idToIndex[source].val].size(); i++)
    {
      if (g.adj_list[idToIndex[source].val][i] == idToIndex[destination].val || g.adj_list[idToIndex[source].val][i] == idToIndex[source].val)
        break;
    }
    if (i != g.adj_list[idToIndex[source].val].size())
      continue;
    g.addEdge(idToIndex[source].val, idToIndex[destination].val);
  }
  graphfile.close();
  ofstream comparer("compare.txt");

  for (it = idToIndex.begin(); it != idToIndex.end(); it++)
  {
    comparer << it->first << " : " << it->second.val << endl;
  }

  struct timeval start, end;
  double time_taken = 0;
  cout << g.e << endl;
  Graph *seq_res = printKCores(g, 3);
  cout<<"hello"<<endl;
  // smallTestCase1();
  // smallTestCase2();
  // smallTestCase3();
  // smallTestCase4();
  for (num = 1; num <= 20; num = num * 2)
  {
    omp_set_num_threads(num);
    gettimeofday(&start, NULL);
    res = compute_k_cores(g, 3);
    gettimeofday(&end, NULL);
    bool error = false;
#pragma omp parallel for default(shared) private(i) reduction(| \
                                                              : error)
    for (i = 0; i < seq_res->adj_list.size(); i++)
    {
      if (seq_res->adj_list[i].size() != res->adj_list[i].size())
      {
        error = true;
      }
    }
    if (error)
    {
      cout << "Error! Incorrect results" << endl;
      // break;
    }
    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec -
                                start.tv_usec)) *
                 1e-6;
    cout << "Threads: " << num << " Time: " << fixed
         << time_taken << setprecision(10) << endl;
    if (num == 16)
      num = 10;
    free(res);
  }
  return 0;
}