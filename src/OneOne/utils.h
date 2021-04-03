/**
 * @file utils.h
 * @author Hrishikesh Athalye & Anup Nair
 * @brief Data structures definitions and APIs
 * @version 0.1
 * @date 2021-04-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @brief Node in the TCB of the thread
 * 
 */
typedef struct node{
    unsigned long int tid;
    unsigned long int tidCpy;
    void* retVal;
    struct node* next;
} node;

/**
 * @brief Singly Linked List of TCBs
 * 
 */
typedef struct singlyLL{
    node *head;
    node *tail;
} singlyLL;

/**
 * @brief Arguments passed to the wrapper function
 * 
 */
typedef struct funcargs{
    void (*f)(void *);
    void* arg;
    node* insertedNode;
    void *stack;
} funcargs;

// Linked List related operations

int singlyLLInit(singlyLL*);

node* singlyLLInsert(singlyLL*, unsigned long int);

int singlyLLDelete(singlyLL*, unsigned long int);

unsigned long int* returnTailTidAddress(singlyLL*);

unsigned long int* returnCustomTidAddress(singlyLL*, unsigned long int);

void persistTid(singlyLL *, unsigned long int);

int killAllThreads(singlyLL*, int signum);

void* getReturnValue(singlyLL *, unsigned long int);



