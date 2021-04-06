#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "../thread.h"
#endif
#include "tests.h"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "../log.h"
thread tid;
thread writerthreads[100],readerthreads[100];
spin_t x,y;
int readercount = 0;

void *reader(void* param)
{
    spin_acquire(&x);
    readercount++;
    if(readercount==1)
        spin_acquire(&y);
    spin_release(&x);
    printf("%d reader is inside\n",readercount);
    usleep(3);
    spin_acquire(&x);
    readercount--;
    if(readercount==0)
    {
        spin_release(&y);
    }
    spin_release(&x);
    printf("%d Reader is leaving\n",readercount+1);
    return NULL;
}

void *writer(void* param)
{
    printf("Writer is trying to enter\n");
    spin_acquire(&y);
    printf("Writer has entered\n");
    spin_release(&y);
    printf("Writer is leaving\n");
    return NULL;
}

int main()
{
    int n2,i;
    printf("Enter the number of readers:");
    scanf("%d",&n2);
    printf("\n");
    int n1[n2];
    spin_init(&x);
    spin_init(&y);
    for(i=0;i<n2;i++)
    {
        thread_create(&writerthreads[i],NULL,reader,NULL);
        thread_create(&readerthreads[i],NULL,writer,NULL);
    }
    for(i=0;i<n2;i++)
    {
        thread_join(writerthreads[i],NULL);
        thread_join(readerthreads[i],NULL);
    }

}