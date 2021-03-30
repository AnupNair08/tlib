typedef struct node{
    unsigned long int tid;
    unsigned long int tidCpy;
    struct node* next;
} node;

typedef struct singlyLL{
    node *head;
    node *tail;
} singlyLL;

int singlyLLInit(singlyLL*);

int singlyLLInsert(singlyLL*, unsigned long int);

int singlyLLDelete(singlyLL*, unsigned long int);

unsigned long int* returnTailTidAddress(singlyLL*);

unsigned long int* returnCustomTidAddress(singlyLL*, unsigned long int);

void persistTid(singlyLL *);

int killAllThreads(singlyLL*, int signum);


typedef struct qnode {
    tcb *tcbnode;
    struct qnode *next;
} qnode;

typedef struct tcbQueue {
    qnode *front;
    qnode *back;
    int len;
} tcbQueue;

int addThread(tcbQueue *, tcb *);
tcb* getNextThread(tcbQueue *);
