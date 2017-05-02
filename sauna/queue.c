#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

/*
*	Very basic implementation of a FIFO queue.
*/

//SECCAO CRITICA
void queue_push(int value, QUEUE *q){
	QNODE *qn = malloc(sizeof(struct qnode));
	
	qn->previous = q->last;
	qn->next = NULL;
	qn->value = value;
	
	q->last->next = qn;
	q->last = qn;

	if(q->first == NULL){
		q->first = qn;
	}
}

//SECCAO CRITICA
int queue_pop(QUEUE *q){
	
	if(q->first == NULL){
		fprintf(stderr,"ERROR! Cannot pop empty queue\n");
		exit(1);
	}
	
	int temp_value = q->first->value;

	if(q->first == q->last){
		free(q->first);
		q->first = NULL;
		q->last = NULL;
	}else{
		q->first = q->first->next;
		free(q->first->previous);
		q->first->previous = NULL;
	}

	return temp_value;
}

void queue_free(QUEUE *q){
	QNODE *qn = q->first;
	while (qn != NULL){
		QNODE *temp = qn->next;
		free(qn);
		qn = temp;
	}
}

void queue_print(QUEUE *q){
	QNODE *qn = q->first;
	printf("Queue: ");
	while(qn != NULL){
		printf("%d ",qn->value);
		qn = qn->next;
	}
	printf("\n");
}
