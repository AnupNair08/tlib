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
tcb* getThread(tcbQueue *, thread);
void printAllmo(tcbQueue *);
void queueRunning(tcbQueue *);
int removeThread(tcbQueue *, unsigned long int);
void reQueue(tcbQueue *, tcb *);