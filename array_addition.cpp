#include <omp.h>
#include <iostream>
#include <vector>

using namespace std;

int main()
{
    
    int n=10000000;
    vector<int> a;
    a.resize(n);
    for(int num=1; num<=20; num++){
	    omp_set_num_threads(num);
	    clock_t start, end;
	    start = clock();
	    int sum = 0,i;
	    // Beginning of parallel region
	    #pragma omp parallel default(shared) private(i)
	    {
		    #pragma omp for 
		    for(int i=0;i<n;i++){
		    int t=rand();
		      a[i]=t;
		    }
		    #pragma omp for reduction(+ : sum)
		    for(int i=0;i<n;i++){
		      sum+=a[i];
		    }
	    }
	    // Ending of parallel region
	    end=clock();
	    double time_taken = double(end-start);
	    cout<<sum<<endl;
	    cout<<"Time: "<<time_taken<<endl;
    }
}
