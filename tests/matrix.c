#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "thread.h"
#endif
#include "tests.h"
#include "attributetypes.h"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "log.h"
mut_t lock;
typedef long ll;
ll **result;
typedef struct arguments{
	ll b1;
	ll row_low;
	ll row_high;
	ll col_low;
	ll col_high;
	ll **mat1;
	ll **mat2;
} arguments;

void accept(ll **mat,ll m, ll n){
	for(ll i = 0; i < m; i++){
		mat[i] = (ll *)malloc(n*sizeof(ll));
	}
	for(ll i = 0 ; i < m ;i++){
		for(ll j = 0 ; j < n; j++){
			scanf("%ld", &mat[i][j]);
		}
	}
	return;
}

void* multiply(void *arg){
	// sleep(1);
	spin_acquire(&lock);
	arguments *arg1 = (arguments *)arg;
	// printf("%ld %ld %ld %ld\n",arg1->row_low,arg1->row_high, arg1->col_low,arg1->col_high);
	for(ll i = arg1->row_low; i < arg1->row_high;i++){
		for(ll j = arg1->col_low; j < arg1->col_high; j++){
			for(ll k = 0 ; k < arg1->b1; k++){
				result[i][j] += arg1->mat1[i][k] * arg1->mat2[k][j]; 
			}
		}
	}
	spin_release(&lock);
	return NULL;
}

void initResult(ll a, ll d){
	result = (ll **)calloc(a,sizeof(ll *));
	for(ll i = 0 ; i < a;i++){
		result[i] = (ll *)calloc(d,sizeof(ll));
	}
}
int testmatrix(){
	spin_init(&lock);
	thread threads[3];
	ll a,b,c,d;

	scanf("%ld %ld",&a,&b);
	ll **m1 = (ll **)malloc(a*sizeof(ll *));
	accept(m1,a,b);
	scanf("%ld %ld",&c,&d);
	if(b != c){
		return -1;
	}
	ll **m2 = (ll **)malloc(c*sizeof(ll *)); 
	accept(m2,c,d);
	initResult(a, d);
	
	arguments temp[3] = {
		{b, 0, a, 0, d/3, m1, m2},
		{b, 0, a, d/3, 2*d/3, m1, m2},
		{b, 0, a, 2*d/3, d, m1, m2}
	};

	for(short i = 0 ; i < 3 ;i++){
		thread_create(&threads[i],NULL,multiply,(void *)&temp[i],0);
		log_trace("Created");
		thread_join(threads[i],NULL);
	}
	int ret;
	for(short i = 0 ; i < 3 ; i++){
		// log_trace("%d\n",ret);
	}
	printf("%ld %ld\n",a,d);
	for(ll i = 0; i < a;i++){
		for(ll j = 0 ; j < d; j++){
			printf("%ld ",result[i][j]);
		}
		if(i != a - 1)
			printf("\n");
	}
}
int main(){
	testmatrix();
	return 0;
}