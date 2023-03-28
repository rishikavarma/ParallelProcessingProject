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

struct Edge
{
  int dest, weight;
  Edge(int d, int w)
  {
    dest = d;
    weight = w;
  }
};

struct Graph
{
  int v = 0, e = 0;
  vector<vector<Edge>> adj_list;
  Graph(int ve)
  {
    v = ve;
    adj_list.resize(v);
  }

  void addEdge(int s, int d, int w)
  {
    Edge te(d, w);
    adj_list[s].push_back(te);
    e++;
  }

  void printDist(int dist[])
  {
    for (int i = 0; i < v; i++)
    {
      cout << i << ": " << dist[i] << endl;
    }
  }
};

int Compute_SSSP(Graph &g, int *dist, int src)
{
  int iter = 0;
  bool *modified = new bool[g.v];
  bool *modified_nxt = new bool[g.v];
#pragma omp parallel for
  for (int t = 0; t < g.v; t++)
  {
    dist[t] = INT_MAX;
    modified[t] = false;
    modified_nxt[t] = false;
  }
  modified[src] = true;
  dist[src] = 0;
  bool finished = false;
  while (!finished)
  {
    iter++;
    if (iter == g.v)
      break;
    finished = true;
#pragma omp parallel
    {
#pragma omp for
      for (int v = 0; v < g.v; v++)
      {
        if (modified[v] == true)
        {
          for (int edge = 0; edge < g.adj_list[v].size(); edge++)
          {
            // cout << v << " " << edge << endl;
            int nbr = g.adj_list[v][edge].dest;
            int e = edge;
            int dist_new = dist[v] + g.adj_list[v][edge].weight;
            bool modified_new = true;
            if (dist[nbr] > dist_new)
            {
              int oldValue = dist[nbr];
              atomicMin(&dist[nbr], dist_new);
              if (oldValue > dist[nbr])
              {
                modified_nxt[nbr] = modified_new;
                finished = false;
              }
            }
          }
        }
      }
#pragma omp for
      for (int v = 0; v < g.v; v++)
      {
        modified[v] = modified_nxt[v];
        modified_nxt[v] = false;
      }
    }
  }
  return iter;
}

int main(int argc, char *argv[])
{
  int v = atoi(argv[2]);
  string str;
  cout << argv[1] << endl;
  ifstream graphfile(argv[1]);
  ofstream expected_output("expected_output_s.txt"), output1("output1_s.txt");
  Graph g(v);
  int s = atoi(argv[3]);
  int num, iter;
  int i, j, k, odist, ind = 0;
  map<int, Index> idToIndex;
  map<int, Index>::iterator it;
  bool change = false;
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
    weight = stoi(word);
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
  int dist_test[v];
  for (i = 0; i < v; i++)
  {
    dist_test[i] = INT_MAX;
  }
  dist_test[s] = 0;
  for (i = 1; i < v; i++)
  {
    change = false;
    for (j = 0; j < g.v; j++)
    {
      for (k = 0; k < g.adj_list[j].size(); k++)
      {
        int u1 = j;
        int u2 = g.adj_list[j][k].dest;
        int w = g.adj_list[j][k].weight;
        if (dist_test[u1] != INT_MAX)
        {
          odist = dist_test[u2];
          dist_test[u2] = min(dist_test[u2], dist_test[u1] + w);
          if (odist != dist_test[u2])
          {
            change = true;
          }
          // cout<<i<<" "<<j<<endl;
        }
      }
    }
    if (!change)
      break;
  }
  for (i = 0; i < v; i++)
  {
    expected_output << i << ": " << dist_test[i] << endl;
  }
  cout << "Sequential run end" << endl;
  for (num = 1; num <= 20; num = num * 2)
  {
    int *dist = new int[v];
    omp_set_num_threads(num);
    gettimeofday(&start, NULL);
    iter = Compute_SSSP(g, dist, s);
    gettimeofday(&end, NULL);
    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec -
                                start.tv_usec)) *
                 1e-6;
    for (i = 0; i < v; i++)
    {
      output1 << i << ": " << dist[i] << endl;
    }
    bool error = false;
#pragma omp parallel for default(shared) private(i) reduction(| \
                                                              : error)
    for (i = 0; i < v; i++)
    {
      if (dist_test[i] != dist[i])
      {
        error = true;
      }
    }
    if (error)
    {
      cout << "Error! Incorrect results" << endl;
      // break;
    }
    cout << "Threads: " << num << " Time: " << fixed
         << time_taken << setprecision(10) << " Iterations: " << iter << endl;
    if (num == 16)
      num = 10;
    delete dist;
  }
  return 0;
}