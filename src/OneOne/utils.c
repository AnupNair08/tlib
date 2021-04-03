#define _GNU_SOURCE
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "utils.h"
#include "log.h"

#define TGKILL 234

//One-One data structure interface

/**
 * @brief Initialize the Singly Linked List
 * 
 * @param ll Pointer to a linked list object
 * @return int On sucess 0, On failure -1
 */
int singlyLLInit(singlyLL *ll){
    if(!ll) return -1;
    ll->head = ll->tail = NULL;
    return 0;
}

/**
 * @brief Insert a node into the linked list
 * 
 * @param ll Pointer to the linked list
 * @param tid Thread ID of the new node
 * @return node* On success Pointer to new node, On failure NULL
 */
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

/**
 * @brief Delete a node from the linked list
 * 
 * @param ll Pointer to the linked list
 * @param tid Thread ID of the node
 * @return int On deletion 0, On not found -1
 */
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
        return -1;
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

/**
 * @brief Get the address of the tail node in the linked list
 * 
 * @param ll Pointer to the linked list
 * @return unsigned long* On sucess address of tail, On failure NULL
 */
unsigned long int* returnTailTidAddress(singlyLL* ll){
    if(ll->head == NULL){
        return NULL;
    }
    return &(ll->tail->tid);
}

/**
 * @brief Get the address of the node with a given tid
 * 
 * @param ll Pointer to linked list
 * @param tid Thread ID of the node
 * @return unsigned long* On sucess address of tail, On failure NULL
 */
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

/**
 * @brief Send process wide signal dispositions to all active threads
 * 
 * @param ll Pointer to linked list
 * @param signum Signal number 
 * @return int On success 0, On failure errno
 */
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

/**
 * @brief Utility function to print the linked list
 * 
 * @param l Pointer to linked list
 */
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

/**
 * @brief Get the Return Value object
 * 
 * @param l Pointer to linked list
 * @param tid Thread ID of the node
 * @return void* On success address of return value, On failure NULL
 */
void* getReturnValue(singlyLL *l,unsigned long int tid){
    node* tmp = l->head;
    while(tmp){ 
        if(tmp->tid == tid){
            return tmp->retVal;
        }
        tmp = tmp->next;
    }
    return NULL;
}