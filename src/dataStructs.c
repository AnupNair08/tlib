#include <stdio.h>
#include <stdlib.h>
#include "dataStructTypes.h"

int singlyLLInit(singlyLL *ll){
    ll->head = ll->tail = NULL;
    return 0;
}

int singlyLLInsert(singlyLL *ll, unsigned long int tid){
    node* tmp;
    // node *tmp1 = ll->head;
    // printf("LL status before ins\n");
    // while(tmp1){
    //     printf("%d\n",tmp1->tid);
    //     tmp1 = tmp1->next;
    // }
    if(posix_memalign((void*)&tmp, 8, sizeof(node))){
        perror("ll alloc");
        return -1;
    }
    int temp = tid;
    tmp->tid = tid;
    tmp->next = NULL;
    if(ll->head==NULL){
        ll->head = ll->tail = tmp;
    }
    else{
        ll->tail->next = tmp;
        ll->tail = tmp;
    }
    return 0;
}

int singlyLLDelete(singlyLL *ll, unsigned long int tid){
    node *tmp1 = ll->head;
    #ifdef DEV
    printf("LL status before del and tid is %d\n",tid)  ;
    while(tmp1){
        printf("%ld ",tmp1->tidCpy);
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

void persistTid(singlyLL* ll){
    ll->tail->tidCpy = ll->tail->tid;
    // printf("%ld\n", ll->tail->tidCpy);
    return;
}