#include <bits/stdc++.h>

using namespace std;

int main(int argc, char *argv[])
{
    string fileName = argv[1];
    int s = atoi(argv[2]);
    int t = atoi(argv[3]);
    ifstream graphfile(fileName + ".txt");
    ofstream randomWeights(fileName + "_withWeights.txt");
    string str;
    set<int> se;
    while (getline(graphfile, str))
    {
        stringstream ss(str);
        string word;
        int source, destination, weight;
        weight = (rand() % 2000);
        ss >> word;
        source = stoi(word);
        ss >> word;
        destination = stoi(word);
        randomWeights << source << "," << destination << "," << weight << "," << endl;
        se.insert(source);
        se.insert(destination);
    }
    int c = 0;
    for (auto i : se)
    {
        if (i != s && i != t && c % 4 == 0)
        {
            int w = rand() % 2000;
            randomWeights << s << "," << i << "," << w << "," << endl;
        }
        else if (i != s && i != t && c % 4 == 2)
        {
            int w = rand() % 2000;
            randomWeights << i << "," << t << "," << w << "," << endl;
        }
        c++;
    }
    randomWeights.close();
    graphfile.close();
    return 0;
}