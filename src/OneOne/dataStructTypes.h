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

node* singlyLLInsert(singlyLL*, unsigned long int);

int singlyLLDelete(singlyLL*, unsigned long int);

unsigned long int* returnTailTidAddress(singlyLL*);

unsigned long int* returnCustomTidAddress(singlyLL*, unsigned long int);

void persistTid(singlyLL *, unsigned long int);

int killAllThreads(singlyLL*, int signum);