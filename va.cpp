#include <omp.h>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <iomanip>

using namespace std;

int main()
{
    
    int n=100000;
    vector<int> a,b,c;
    a.resize(n);
    b.resize(n);
    c.resize(n);
	int i =0;
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
    for(int num=1;num<=10;num++){
	    omp_set_num_threads(num);
	    struct timeval start, end;
	   	gettimeofday(&start, NULL);
	    // Beginning of parallel region	    
	    #pragma omp parallel for default(shared) private(i) 
	    for(i=0;i<n;i++){
	      c[i]=a[i]+b[i];
	    }
	    // Ending of parallel region
	    gettimeofday(&end, NULL);
		double time_taken=0;
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - 
								start.tv_usec)) * 1e-6;
	    /*for(int i=0;i<n;i++){
	      cout<<c[i]<<" ";
	    }*/
	    cout<<endl;
	    cout<<"Time: "<<fixed
         << time_taken << setprecision(6)<<endl;
	}
}
