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

//Many-One data structure interface

int addThread(tcbQueue *t, tcb *thread_tcb){
    qnode *temp = (qnode *)malloc(sizeof(qnode));
    if(!temp) return -1;
    temp->tcbnode = thread_tcb;
    temp->next = NULL;

    if(t->front == NULL){
        t->front = temp;
        t->back = temp;
        t->len++;
        return 0;
    }
    t->back->next = temp;
    t->back = temp;
    t->len++;
    return 0;
}

tcb* getNextThread(tcbQueue *t){
        // log_trace("%ld",getpid());
    qnode *temp = t->front;
    if(temp == NULL){
        return NULL;
    }
    if(temp->next == NULL){
        if(temp->tcbnode->thread_state == RUNNABLE){
            return temp->tcbnode;
        }
        else{
            return NULL;
        }
    }
    if(temp->tcbnode->thread_state == RUNNABLE){
        struct tcb* retTcb =  temp->tcbnode;
        t->front = t->front->next;
        addThread(t, retTcb);
        free(temp);
        return retTcb;
    }
    while(temp->next){
        struct qnode* delNode =  temp->next;
        if(delNode->tcbnode->thread_state == RUNNABLE){
            temp->next = delNode->next;
            struct tcb* retTcb = delNode->tcbnode;
            if(delNode == t->back){
                t->back = temp;
            }
            addThread(t, retTcb);
            free(delNode);
            return retTcb;
        }
        temp = temp->next;
    }
    return NULL;
}

tcb* getThread(tcbQueue *t, thread tid){
    if(t->front){
        qnode *temp = t->front;
        while(temp!=NULL){
            if(temp->tcbnode->tid == tid){
                return temp->tcbnode;
            }
            temp = temp->next;
        }
    }
    return NULL;
}

void printAllmo(tcbQueue *t){
    qnode *q = t->front;
    while(q!=NULL){
        printf("%d, %d -> ", q->tcbnode->tid, q->tcbnode->thread_state);
        q = q->next;
    }
    printf("\n");
    return;
}

void queueRunning(tcbQueue *t){
    qnode *temp = t->front;
    if(temp == NULL){
        return;
    }
    if(temp->tcbnode->thread_state == RUNNING){
        struct tcb* retTcb =  temp->tcbnode;
        t->front = t->front->next;
        addThread(t, retTcb);
        free(temp);
        return;
    }
    while(temp->next){
        struct qnode* delNode =  temp->next;
        if(delNode->tcbnode->thread_state == RUNNING){
            temp->next = delNode->next;
            struct tcb* retTcb = delNode->tcbnode;
            if(delNode == t->back){
                t->back = temp;
            }
            addThread(t, retTcb);
            free(delNode);
            break;
        }
        temp = temp->next;
    }
    return;
}