#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

/*
*	Very basic implementation of a FIFO queue with possibility of mutex usage for synchronization.
*   (It's just a linked list with shortcuts to the first and last element and push and pop functions that match the desired functionality of a queue.)
*/

void queueMutexPush(void *data, QUEUE *q, pthread_mutex_t *mut){
    
    if(pthread_mutex_lock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }
    
    queuePush(data, q);
    
    if(pthread_mutex_unlock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }
}

void * queueMutexPop(QUEUE *q, pthread_mutex_t *mut){
	void * temp;

    if(pthread_mutex_lock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }
    
    temp = queuePop(q);
    
    if(pthread_mutex_unlock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }
    
    return temp;
}

void queuePush(void * data, QUEUE *q){
	QNODE *qn = malloc(sizeof(struct qnode));
	
	qn->previous = q->last;
	qn->next = NULL;
	qn->data = data;

	if (q->last != NULL) 
		q->last->next = qn;
	
	q->last = qn;

	if(q->first == NULL){
		q->first = qn;
	}
}

void *queuePop(QUEUE *q){
	
	if(q->first == NULL){
		fprintf(stderr,"QUEUE ERROR! Cannot pop empty queue.\n");
		exit(1);
	}
	
	void *temp_data = q->first->data;
	

	if(q->first == q->last){
		free(q->first);
		q->first = NULL;
		q->last = NULL;
	}else{
		q->first = q->first->next;
		free(q->first->previous);
		q->first->previous = NULL;
	}

	return temp_data;
}

void queueFree(QUEUE *q){
	QNODE *qn = q->first;
	
	while (qn != NULL){
		QNODE *temp = qn->next;
		if (q->dynamic) 
			free(qn->data);
		free(qn);
		qn = temp;
	
	}
	q->first = NULL;
	q->last = NULL;
}

void queueMutexFree(QUEUE *q, pthread_mutex_t *mut){
	
	if(pthread_mutex_lock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }
	
	queueFree(q);
	
	if(pthread_mutex_unlock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }
}

int queueIsEmpty(QUEUE *q){
	if(q->first == NULL){
		return 1;
	}else{
		return 0;
	}
} 

int queueMutexIsEmpty(QUEUE *q, pthread_mutex_t *mut){
	int temp = 0;

	if(pthread_mutex_lock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }
	
	temp = queueIsEmpty(q);
	
	if(pthread_mutex_unlock(mut)){
    	perror("MUTEX LOCK ERROR");
    	exit(1);
    }

    return temp;
}
