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

int removeThread(tcbQueue *t, unsigned long int tid){
    qnode * tmp = t->front;
    if(t->front == NULL){
        return 0;
    }
    if(t->front->tcbnode->tid == tid){
        t->front = tmp->next;
        if(t->front == NULL){
            t->back = NULL;
        }
        // if(tmp->tcbnode->stack){
        //     free(tmp->tcbnode->stack);
        // }
        free(tmp->tcbnode->context);
        free(tmp->tcbnode->waiters);
        free(tmp->tcbnode);
        free(tmp);
        t->len--;
        return 0;
    }
    else{
        if(tmp->next == NULL){
            return 0;
        }
        while(tmp->next){
            if(tmp->next->tcbnode->tid == tid){
                qnode* delNode = tmp->next;
                if(delNode == t->back){
                    t->back = tmp;
                }
                tmp->next = delNode->next;
                // if(tmp->tcbnode->stack){
                //     free(tmp->tcbnode->stack);
                // }
                free(delNode->tcbnode->context);
                free(delNode->tcbnode->waiters);
                free(delNode->tcbnode);
                free(delNode);
                t->len--;
                return 0;
            }
            tmp = tmp->next;
        }
    }
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
        printf("%ld, %d -> ", q->tcbnode->tid, q->tcbnode->thread_state);
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

void reQueue(tcbQueue *t, tcb* tcb){
    qnode *temp = t->front;
    if(temp == NULL){
        return;
    }
    if(temp->tcbnode->tid == tcb->tid){
        t->front = t->front->next;
        addThread(t, tcb);
        free(temp);
        return;
    }
    while(temp->next){
        struct qnode* delNode =  temp->next;
        if(delNode->tcbnode->tid == tcb->tid){
            temp->next = delNode->next;
            if(delNode == t->back){
                t->back = temp;
            }
            addThread(t, tcb);
            free(delNode);
            break;
        }
        temp = temp->next;
    }
    return;
}