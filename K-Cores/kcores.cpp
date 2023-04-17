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
    min_degree = min(degree[s], degree[d], min_degree);
    if (min_degree == degree[s])
      start_vertex = s;
    if (min_degree == degree[d])
      start_vertex = d;
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
    if (!visited[adj_v])
    {
      if (g.degree[adj_v] < k)
      {
        for (int j = 0; j < g.degree[adj_v]; j++)
        {
          if (g.adj_list[adj_v][j] == v)
          {
            g.adj_list[adj_v][j] = g.adj_list[adj_v][g.degree[adj_v]];
            break;
          }
        }
        compute_k_cores_dfs(g, visited, adj_v, k);
      }
    }
  }
}

void compute_k_cores(Graph &g, int k)
{
  vector<bool> visited(g.v, false);
  compute_k_cores_dfs(g, visited, g.start_vertex, k);
  for (int i = 0; i < g.v; i++)
  {
    if (!visited[i] && g.degree[i] < k)
    {
      compute_k_cores_dfs(g, visited, i, k);
    }
  }
}

void main(int argc, char *argv[])
{
}