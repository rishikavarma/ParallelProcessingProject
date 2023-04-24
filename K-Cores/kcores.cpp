#include <bits/stdc++.h>
#include <omp.h>
#include <sys/time.h>
#include <atomic>

using namespace std;

ofstream output1("output1_r.txt"),output2("output2_r.txt");
template <typename T>
inline void atomicMin(T *targetVar, T update_val)
{
  T oldVal, newVal;
  do
  {
    oldVal = *targetVar;
    newVal = std::min(oldVal, update_val);
    if (oldVal == newVal)
      break;
  } while (__sync_val_compare_and_swap(targetVar, oldVal, newVal) == false);
}

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
       output2  << i << "  :  ";
        for (int j = 0; j < adj_list[i].size(); j++)
        {
          if (degree[adj_list[i][j]] >= k)
          {
            output2 << " " << adj_list[i][j] << ",";
          }
        }
        output2 << endl;
      }
    }
  }

  void printGraph()
  {
    for (int i = 0; i < v; i++)
    {
      if (adj_list[i].size())
      {
        output2 << i << "  :  ";
        for (int j = 0; j < adj_list[i].size(); j++)
        {
          output2 << " " << adj_list[i][j] << ",";
        }
        output2 << endl;
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
  for (int i = 0; i < g.adj_list[i].size(); i++)
  {
    int adj_v = g.adj_list[v][i];
    g.degree[adj_v]--;
    if (!visited[adj_v] && g.degree[adj_v] < k)
    {
      // output1 << adj_v << endl;
      compute_k_cores_dfs(g, visited, adj_v, k);
    }
  }
}

Graph* compute_k_cores(Graph &g, int k)
{
  int *visited = new int[g.v];
  int i;
#pragma omp parallel for default(shared) private(i)
  for (i = 0; i < g.v; i++)
  {
    if (!visited[i] && g.degree[i] < k)
    {
      // output1 << i << endl;
      compute_k_cores_dfs(g, visited, i, k);
    }
  }
  Graph *kCore = new Graph(g.v);
  // #pragma omp parallel for default(shared) private(i)
  for (i = 0; i < g.v; i++)
  {
    if (g.degree[i] >= k)
    {
      for (int j = 0; j < g.adj_list[i].size(); j++)
      {
        if (g.degree[g.adj_list[i][j]] >= k)
        {
          kCore->addDirectedEdge(i, g.adj_list[i][j]);
        }
      }
    }
  }
  kCore->printGraph();
  return kCore;
}

int main(int argc, char *argv[])
{
  Graph *res;
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
  output2 << "G1:" << endl;
  // output1 <<"G1:" << endl;
  res = compute_k_cores(g1, 3);
  vector<int> ans{2,3,4,6,7}; 
  for(int i=0;i<ans.size();i++){
    if(res->adj_list[ans[i]].size()==0){
      cout<<"G1 error"<<endl;
    }
  }
  free(res);

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
  output2 << "G2:" << endl;
  // output1 << "G2:" << endl;
  res = compute_k_cores(g2, 3);
  for(int i=0;i<res->adj_list.size();i++){
    if(res->adj_list[i].size()!=0){
      cout<<"G2 error"<<endl;
    }
  }
  free(res);

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
  output2 << "G3:" << endl;
  // output1 << "G3:" << endl;
  res = compute_k_cores(gr, 3);
  vector<int> ans3{2,3,4,6}; 
  for(int i=0;i<ans3.size();i++){
    if(res->adj_list[ans3[i]].size()==0){
      cout<<"G3 error"<<endl;
    }
  }
  free(res);

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
  output2 << "G4:" << endl;
  // output1 << "G4:" << endl;
  res = compute_k_cores(g4, 3);
  vector<int> ans4{1,2,3,4,5,6}; 
  for(int i=0;i<ans4.size();i++){
    if(res->adj_list[ans4[i]].size()==0){
      cout<<"G4 error"<<endl;
    }
  }
  free(res);
  return 0;
}