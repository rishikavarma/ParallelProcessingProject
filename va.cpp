#include <omp.h>
#include <iostream>
#include <vector>

using namespace std;

int main()
{
    
    int n=100000;
    vector<int> a,b,c;
    a.resize(n);
    b.resize(n);
    c.resize(n);
    for(int num=1;num<=10;num++){
	    omp_set_num_threads(num);
	    int i =0;
	    clock_t start, end;
	    start = clock();
	    // Beginning of parallel region
	    #pragma omp parallel default(shared) private(i) 
	    {
		    #pragma omp for nowait
		    for(i=0;i<n;i++){
		      int t=rand();
		      a[i]=t;
		    }
		    #pragma omp for nowait
		    for(i=0;i<n;i++){
		      int t=rand();
		      b[i]=t;
		    }
	    }
	    #pragma omp parallel for default(shared) private(i) 
	    for(i=0;i<n;i++){
	      c[i]=a[i]+b[i];
	    }
	    // Ending of parallel region
	    end=clock();
	    double time_taken = double(end-start);
	    /*for(int i=0;i<n;i++){
	      cout<<c[i]<<" ";
	    }*/
	    cout<<endl;
	    cout<<"Time: "<<time_taken<<endl;
	}
}
