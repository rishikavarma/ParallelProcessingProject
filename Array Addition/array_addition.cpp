#include <omp.h>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <iomanip>

using namespace std;

int main()
{
    
    int n=10000000;
    vector<int> a;
    a.resize(n);
    for(int num=1; num<=20; num++){
	    omp_set_num_threads(num);
	    struct timeval start, end;
	   	gettimeofday(&start, NULL);
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
	   	gettimeofday(&end, NULL);
		double time_taken=0;
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - 
								start.tv_usec)) * 1e-6;
	    cout<<sum<<endl;
	    cout<<"Time: "<<fixed
         << time_taken << setprecision(6)<<endl;
    }
}
