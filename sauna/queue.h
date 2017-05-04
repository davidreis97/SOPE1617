#include <pthread.h>

typedef struct qnode{
	void *data;
	struct qnode *previous;
	struct qnode *next;
}QNODE;

typedef struct queue{
	QNODE *first;
	QNODE *last;
	//int shared;
	int dynamic; //This flag can be used to tell the queue to automatically free() the content of each node.
}QUEUE;

void queuePush(void *value, QUEUE *q);

void queueMutexPush(void *value, QUEUE *queue, pthread_mutex_t *mut);

void *queuePop(QUEUE *q);

void *queueMutexPop(QUEUE *queue, pthread_mutex_t *mut);	

int queueIsEmpty(QUEUE *q);

int queueMutexIsEmpty(QUEUE *q, pthread_mutex_t *mut);

void queueFree(QUEUE *q);

void queueMutexFree(QUEUE *q, pthread_mutex_t *mut);
