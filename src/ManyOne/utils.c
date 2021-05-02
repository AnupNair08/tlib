/**
 * @file utils.c
 * @author Hrishikesh Athalye & Anup Nair
 * @brief Data Structures for implementing library interface
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include "utils.h"

#define TGKILL 234

//Many-One data structure interface

int addThread(tcbQueue *t, tcb *thread_tcb)
{
    qnode *temp = (qnode *)malloc(sizeof(qnode));
    if (!temp)
        return -1;
    temp->tcbnode = thread_tcb;
    temp->next = NULL;

    if (t->front == NULL)
    {
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

int removeThread(tcbQueue *t, unsigned long int tid)
{
    thread *reQueueList = NULL;
    int numRequeue = 0;
    qnode *tmp = t->front;
    if (t->front == NULL)
    {
        return 0;
    }
    if (t->front->tcbnode->tid == tid)
    {
        for (int j = 0; j < t->front->tcbnode->numWaiters; j++)
        {
            reQueueList = (thread *)realloc(reQueueList, ++numRequeue * sizeof(thread));
            reQueueList[numRequeue - 1] = t->front->tcbnode->waiters[j];
        }
        t->front = tmp->next;
        if (munmap(tmp->tcbnode->stack, STACK_SZ + getpagesize()))
        {
            return errno;
        }
        if (t->front == NULL)
        {
            t->back = NULL;
        }
        free(tmp->tcbnode->waiters);
        free(tmp->tcbnode);
        free(tmp);
        t->len--;
        for (int i = 0; i < numRequeue; i++)
        {
            tcb *setRunnable = getThread(t, reQueueList[i]);
            setRunnable->thread_state = RUNNABLE;
            reQueue(t, setRunnable);
        }
        free(reQueueList);
        return 0;
    }
    else
    {
        if (tmp->next == NULL)
        {
            return 0;
        }
        while (tmp->next)
        {
            if (tmp->next->tcbnode->tid == tid)
            {
                for (int j = 0; j < tmp->next->tcbnode->numWaiters; j++)
                {
                    reQueueList = (thread *)realloc(reQueueList, ++numRequeue * sizeof(thread));
                    reQueueList[numRequeue - 1] = tmp->next->tcbnode->waiters[j];
                }
                if (munmap(tmp->next->tcbnode->stack, STACK_SZ + getpagesize()))
                {
                    return errno;
                }
                qnode *delNode = tmp->next;
                if (delNode == t->back)
                {
                    t->back = tmp;
                }
                tmp->next = delNode->next;
                free(delNode->tcbnode->waiters);
                free(delNode->tcbnode);
                free(delNode);
                t->len--;
                for (int i = 0; i < numRequeue; i++)
                {
                    tcb *setRunnable = getThread(t, reQueueList[i]);
                    setRunnable->thread_state = RUNNABLE;
                    reQueue(t, setRunnable);
                }
                free(reQueueList);
                return 0;
            }
            tmp = tmp->next;
        }
    }
    return 0;
}

tcb *getNextThread(tcbQueue *t)
{
    qnode *temp = t->front;
    if (temp == NULL)
    {
        return NULL;
    }
    if (temp->next == NULL)
    {
        if (temp->tcbnode->thread_state == RUNNABLE)
        {
            return temp->tcbnode;
        }
        else
        {
            return NULL;
        }
    }
    if (temp->tcbnode->thread_state == RUNNABLE)
    {
        struct tcb *retTcb = temp->tcbnode;
        t->front = t->front->next;
        addThread(t, retTcb);
        free(temp);
        return retTcb;
    }
    while (temp->next)
    {
        struct qnode *delNode = temp->next;
        if (delNode->tcbnode->thread_state == RUNNABLE)
        {
            temp->next = delNode->next;
            struct tcb *retTcb = delNode->tcbnode;
            if (delNode == t->back)
            {
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

tcb *getThread(tcbQueue *t, thread tid)
{
    qnode *temp = t->front;
    while (temp != NULL)
    {
        if (temp->tcbnode->tid == tid)
        {
            return temp->tcbnode;
        }
        temp = temp->next;
    }
    return NULL;
}

void printAllmo(tcbQueue *t)
{
    qnode *q = t->front;
    while (q != NULL)
    {
        printf("%ld, %d -> ", q->tcbnode->tid, q->tcbnode->thread_state);
        q = q->next;
    }
    printf("\n");
    return;
}

void queueRunning(tcbQueue *t)
{
    qnode *temp = t->front;
    if (temp == NULL)
    {
        return;
    }
    if (temp->tcbnode->thread_state == RUNNING)
    {
        struct tcb *retTcb = temp->tcbnode;
        t->front = t->front->next;
        retTcb->thread_state = RUNNABLE;
        addThread(t, retTcb);
        free(temp);
        return;
    }
    while (temp->next)
    {
        struct qnode *delNode = temp->next;
        if (delNode->tcbnode->thread_state == RUNNING)
        {
            temp->next = delNode->next;
            struct tcb *retTcb = delNode->tcbnode;
            if (delNode == t->back)
            {
                t->back = temp;
            }
            retTcb->thread_state = RUNNABLE;
            addThread(t, retTcb);
            free(delNode);
            break;
        }
        temp = temp->next;
    }
    return;
}

void reQueue(tcbQueue *t, tcb *tcb)
{
    qnode *temp = t->front;
    if (temp == NULL)
    {
        return;
    }
    if (temp->tcbnode->tid == tcb->tid)
    {
        t->front = t->front->next;
        addThread(t, tcb);
        free(temp);
        return;
    }
    while (temp->next)
    {
        struct qnode *delNode = temp->next;
        if (delNode->tcbnode->tid == tcb->tid)
        {
            temp->next = delNode->next;
            if (delNode == t->back)
            {
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

void removeExitedThreads(tcbQueue *t)
{
    thread *deleted = NULL;
    int numDeleted = 0;
    qnode *tmp = t->front;
    while (tmp)
    {
        if (tmp->tcbnode->exited == 1)
        {
            deleted = (thread *)realloc(deleted, ++numDeleted * sizeof(thread));
            deleted[numDeleted - 1] = tmp->tcbnode->tid;
        }
        tmp = tmp->next;
    }
    for (int i = 0; i < numDeleted; i++)
    {
        removeThread(t, deleted[i]);
    }
    free(deleted);
    return;
}

void unlockMutex(tcbQueue *t, mutex_t *lock)
{
    qnode *tmp = t->front;
    tcb **requeue = NULL;
    int numRequeue = 0;
    while (tmp)
    {
        if (tmp->tcbnode->mutexWait == lock)
        {
            tmp->tcbnode->mutexWait = NULL;
            tmp->tcbnode->thread_state = RUNNABLE;
            requeue = (tcb **)realloc(requeue, ++numRequeue);
            requeue[numRequeue - 1] = tmp->tcbnode;
            //Once the first thread that was waiting is found, break
            break;
        }
        tmp = tmp->next;
    }
    for (int i = 0; i < numRequeue; i++)
    {
        reQueue(t, requeue[i]);
    }
    free(requeue);
    return;
}

void initTcb(tcb *t, int initState, thread tid, sigjmp_buf *context)
{
    t->thread_state = initState;
    t->tid = tid;
    t->exited = 0;
    t->waiters = NULL;
    t->numWaiters = 0;
    t->numPendingSig = 0;
    t->pendingSig = NULL;
    t->mutexWait = NULL;
    t->ctx = context;
    t->stack = NULL;
}