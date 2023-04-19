#include <bits/stdc++.h>
#include <omp.h>
#include <sys/time.h>
#include <atomic>

using namespace std;

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

  void addEdge(int s, int d)
  {
    adj_list[s].push_back(d);
    adj_list[d].push_back(s);
    degree[s]++;
    degree[d]++;
    e++;
  }

  void printKCoresGraph(int k)
  {
    for (int i = 0; i < v; i++)
    {
      if (degree[i] >= k)
      {
        cout << i << "  :  ";
        for (int j = 0; j < degree[i]; j++)
        {
          if (degree[adj_list[i][j]] >= k)
          {
            cout << " " << adj_list[i][j] << ",";
          }
        }
        cout << endl;
      }
    }
  }
};

void compute_k_cores_dfs(Graph &g, vector<bool> &visited, int v, int k)
{
  visited[v] = true;
  for (int i = 0; i < g.degree[v]; i++)
  {
    int adj_v = g.adj_list[v][i];
    g.degree[adj_v]--;
    for (int j = 0; j < g.degree[adj_v]; j++)
    {
      if (g.adj_list[adj_v][j] == v)
      {
        g.adj_list[adj_v][j] = g.adj_list[adj_v][g.degree[adj_v]];
        break;
      }
    }
    // if(!visited[adj_v]) cout<<adj_v<<endl;
    if (!visited[adj_v] && g.degree[adj_v] < k)
    {
      compute_k_cores_dfs(g, visited, adj_v, k);
    }
  }
}

void compute_k_cores(Graph &g, int k)
{
  vector<bool> visited(g.v, false);
  for (int i = 0; i < g.v; i++)
  {
    //  if(!visited[i]) cout<<i<<endl;
    if (!visited[i] && g.degree[i] < k)
    {

      compute_k_cores_dfs(g, visited, i, k);
    }
  }
}

int main(int argc, char *argv[])
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
  compute_k_cores(g1, 3);
  cout << "G1:" << endl;
  g1.printKCoresGraph(3);

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
  compute_k_cores(g2, 3);
  cout << "G2:" << endl;
  g2.printKCoresGraph(3);

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
  compute_k_cores(gr, 3);
  cout << "G3:" << endl;
  gr.printKCoresGraph(3);
  return 0;
}