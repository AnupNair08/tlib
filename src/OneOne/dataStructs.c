#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "tlibtypes.h"
#include "dataStructTypes.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "log.h"

#define TGKILL 234

//One-One data structure interface

int singlyLLInit(singlyLL *ll){
    ll->head = ll->tail = NULL;
    return 0;
}

node* singlyLLInsert(singlyLL *ll, unsigned long int tid){
    node* tmp;
    if(posix_memalign((void**)&tmp, 8, sizeof(node))){
        perror("ll alloc");
        return NULL;
    }
    tmp->tid = tid;
    tmp->next = NULL;
    if(ll->head==NULL){
        ll->head = ll->tail = tmp;
    }
    else{
        ll->tail->next = tmp;
        ll->tail = tmp;
    }
    return tmp;
}

int singlyLLDelete(singlyLL *ll, unsigned long int tid){
    node *tmp1 = ll->head;
    #ifdef DEV
    printf("LL status before del and tid is %d\n",tid)  ;
    while(tmp1){
        log_error("%ld ",tmp1->tidCpy);
        tmp1 = tmp1->next;
    }
    puts("");
    #endif // !DEV
    node* tmp = ll->head;
    if(tmp == NULL){
        return 0;
    }
    if(tmp->tidCpy == tid){
        ll->head = ll->head->next;
        free(tmp);
        if(ll->head == NULL){
            ll->tail = NULL;
        }
        return 0;
    }
    while(tmp->next){
        if(tmp->next->tidCpy == tid){
            node* tmpNext = tmp->next->next;
            if(tmp->next == ll->tail){
                ll->tail = tmp;
            }
            free(tmp->next);
            tmp->next = tmpNext;
            break;
        }
        tmp = tmp->next;
    }
    return 0;
}

unsigned long int* returnTailTidAddress(singlyLL* ll){
    if(ll->head == NULL){
        return NULL;
    }
    return &(ll->tail->tid);
}

unsigned long int* returnCustomTidAddress(singlyLL* ll, unsigned long int tid){
    node* tmp = ll->head;
    while(tmp!=NULL){
        if(tmp->tidCpy == tid){
            return &(tmp->tid);
        }
        tmp = tmp->next;
    }
    return NULL;
}

int killAllThreads(singlyLL* ll,int signum){
    node* tmp = ll->head;
    pid_t pid = getpid();
    int ret;
    pid_t delpid[100];
    int counter = 0;
    while(tmp){
        if(tmp->tid == gettid()) {
            tmp = tmp->next;
            continue;
        }
        printf("Killed thread %ld\n",tmp->tid);
        ret = syscall(TGKILL, pid, tmp->tid, signum);
        if(ret == -1){
            perror("tgkill");
            return errno;
        }
        else{
            if(signum == SIGINT || signum == SIGKILL){
                delpid[counter++] = tmp->tid;
            }
        }
        tmp = tmp->next;
    }
    if(signum == SIGINT || signum == SIGKILL){
        for(int i= 0 ; i < counter ;i++) singlyLLDelete(ll,delpid[i]);
    }
    return 0;
}

void printAllNodes(singlyLL *l){
    node* tmp = l->head;
    while(tmp){
        printf("tid%ld tidCpy%ld-->",  tmp->tid, tmp->tidCpy);
        fflush(stdout);
        tmp = tmp->next;
    }
    printf("\n");
    return;
}

void* getReturnValue(singlyLL *l, thread tid){
    node* tmp = l->head;
    while(tmp){ 
        if(tmp->tid == tid){
            return tmp->retVal;
        }
        tmp = tmp->next;
    }
    return NULL;
}