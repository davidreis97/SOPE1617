typedef struct qnode{
	int value;
	struct qnode *previous;
	struct qnode *next;
}QNODE;

typedef struct queue{
	QNODE *first;
	QNODE *last;
}QUEUE;

void queue_push(int value, QUEUE *q);

int queue_pop(QUEUE *q);

void queue_free(QUEUE *q);

void queue_print(QUEUE *q);
