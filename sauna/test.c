#include <pthread.h>
#include "constants.h"
#include "queue.h"

pthread_mutex_t requestsQueueMutex = PTHREAD_MUTEX_INITIALIZER;

QUEUE requestsQueue;

void main(){
	
	REQUEST *req = malloc(sizeof(struct request_info));

    if(rand()%2 == 0)
        req->gender = 'F';
    else
        req->gender = 'M';
    req->serialNum = 12;
    req->rejections = 0;
    req->time = rand()%100;
        
    queuePush(req, &requestsQueue);

    REQUEST reqs = *(REQUEST *)queuePop(&requestsQueue);

    printf("SENT REQUEST: %s - %d - %d: %c - %d - %s\n","Agora", getpid(), reqs.serialNum, reqs.gender, reqs.time, "PEDIDO");
    
}
