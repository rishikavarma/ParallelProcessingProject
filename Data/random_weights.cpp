#include <bits/stdc++.h>

using namespace std;

int main(){
    string fileName = "email-EuAll";
    ifstream graphfile(fileName+".txt");
    ofstream randomWeights(fileName+"_withWeights.txt");
    string str;
    while (getline (graphfile, str)){
		stringstream ss(str);
        string word;
		int source,destination,weight;
        weight = (rand()%2000);
		ss>>word;
        source = stoi(word);
		ss>>word;
        destination = stoi(word);
        randomWeights<<source<<","<<destination<<","<<weight<<","<<endl;
	}
    return 0;
}