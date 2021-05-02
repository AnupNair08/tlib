/**
 * @file utils.h
 * @author Hrishikesh Athalye & Anup Nair
 * @brief Data Structures used for thread book keeping
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "tlib.h"

/**
 * @brief Structure representing a thread
 * 
 */
typedef struct tcb
{
    thread tid;
    void *stack;
    size_t stack_sz;
    int thread_state;
    sigjmp_buf *ctx;
    int exited;   //indicate if the process has exited or not
    int *waiters; //list of all waiters on this process
    int numWaiters;
    mutex_t *mutexWait;
    int *pendingSig;
    int numPendingSig;
    funcargs *args;
} tcb;

/**
 * @brief Node representing a TCB in the queue
 * 
 */
typedef struct qnode
{
    tcb *tcbnode;
    struct qnode *next;
} qnode;

/**
 * @brief Queue of TCBs
 * 
 */
typedef struct tcbQueue
{
    qnode *front;
    qnode *back;
    int len;
} tcbQueue;

/**
 * @brief Initialzes the given TCB 
 * @param 
 */
void initTcb(tcb *, int, thread, sigjmp_buf *);

/**
 * @brief 
 * 
 * @return int 
 */
int addThread(tcbQueue *, tcb *);
/**
 * @brief Get the Next Thread object
 * 
 * @return tcb* 
 */
tcb *getNextThread(tcbQueue *);
/**
 * @brief Get the Thread object
 * 
 * @return tcb* 
 */
tcb *getThread(tcbQueue *, thread);
/**
 * @brief 
 * 
 */
void printAllmo(tcbQueue *);
/**
 * @brief 
 * 
 */
void queueRunning(tcbQueue *);
/**
 * @brief 
 * 
 */
void removeExitedThreads(tcbQueue *);
/**
 * @brief 
 * 
 */
void reQueue(tcbQueue *, tcb *);
/**
 * @brief 
 * 
 */
void unlockMutex(tcbQueue *, mutex_t *);
