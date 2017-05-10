#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "queue.h"
#include "constants.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/times.h>

typedef struct command{
    int requests;
    int maxTime;
} COMMAND;

COMMAND command;

clock_t start = 0;

FILEDESCRIPTORS fds;

QUEUE requests;

pthread_mutex_t requestsMutex = PTHREAD_MUTEX_INITIALIZER;

int endRejection = 0;

double getTime() {
    clock_t end;
    struct tms t;
    long ticks;

    if(start == 0){
        start = times(&t);
    }

    ticks = sysconf(_SC_CLK_TCK);

    end = times(&t);

    return ((double)(end-start) * 1000/ticks);
}

void* rejectionHandler(void* arg){

    REQUEST temp;
    int i = 0;
    char message[512];

    while((i = read(fds.fifoRejected, &temp, sizeof(struct request_info)))){

        memset(message, '\0', 512*sizeof(char));

        if(i == -1){
            printf("File Desc: %d",fds.fifoRejected);
            perror("READ ERROR");
            pthread_exit(NULL);
        }

        REQUEST* req = malloc(sizeof(struct request_info));

        *req = temp;

        req->rejections++;

        sprintf(message,"%f - %9.9d - %9.9d: %c - %9.9d - REJEITADO\n",getTime(), getpid(), req->serialNum, req->gender, req->time);
        write(fds.fileLog, message, sizeof(char)*512);

        if(req->rejections >= 3){
            sprintf(message,"%f - %9.9d - %9.9d: %c - %9.9d - DESCARTADO\n",getTime(), getpid(), req->serialNum, req->gender, req->time);
            write(fds.fileLog, message, sizeof(char)*512);
            
            free(req);
        }
        else{
            queueMutexPush(req, &requests, &requestsMutex);
        }

    }

    endRejection = 1;

    return NULL;
}

void argumentHandling(int argc, char*argv[]){    
    if (argc != 3){
        printf("Usage: %s <requests> <max. time>\n",argv[0]);
        exit(1);
    }

    command.requests = atoi(argv[1]);

    if(command.requests <= 0){
         fprintf(stderr, "Error! Number of requests must be a positive integer.\n");
    }

    command.maxTime = atoi(argv[2]);

    if(command.maxTime <= 0){
         fprintf(stderr, "Error! Maximum time must be a positive integer.\n");
    }
}

void initCommunications(){

    char filename[20];
    sprintf(filename, "/tmp/ger.%d", getpid());

    if((fds.fileLog = open(filename, O_WRONLY | O_CREAT | O_EXCL, OP_MODE)) < 0){
        perror("Couldn't create file_info ");
        exit(EXIT_FAILURE);
    }

    /* Creates FIFO's "entrada" e "rejeitados" */

    if(mkfifo("/tmp/entrada", OP_MODE) < 0){
        perror("Couldn't create FIFO '/tmp/entrada' ");
        exit(EXIT_FAILURE);
    }

    if(mkfifo("/tmp/rejeitados", OP_MODE) < 0){
        perror("Couldn't create FIFO '/tmp/rejeitados' ");
        exit(EXIT_FAILURE);
    }

    if((fds.fifoRequests = open("/tmp/entrada", O_WRONLY)) < 0){
        perror("Couldn't open FIFO '/tmp/entrada' ");
        exit(EXIT_FAILURE); 
    }

    if((fds.fifoRejected = open("/tmp/rejeitados", O_RDONLY)) < 0){
        perror("Couldn't open FIFO '/tmp/rejeitados' ");
        exit(EXIT_FAILURE);
    }

}

void closeCommunications(){

    if(close(fds.fileLog) < 0){
        perror("Couldn't close file ");
        exit(EXIT_FAILURE);
    }

    if(close(fds.fifoRequests) < 0){
        perror("Couldn't close '/tmp/entrada' ");
        exit(EXIT_FAILURE);
    }

    if(close(fds.fifoRejected) < 0){
        perror("Couldn't close '/tmp/rejeitados' ");
        exit(EXIT_FAILURE);
    }

    if(unlink("/tmp/entrada") < 0){
        perror("Unlinking '/tmp/entrada' error ");
        exit(EXIT_FAILURE);
    }

    if(unlink("/tmp/rejeitados") < 0){
        perror("Unlinking '/tmp/rejeitados' error ");
        exit(EXIT_FAILURE);
    }
}

void sendRequests(){
    char message[512];

    while(1){
        while(!queueMutexIsEmpty(&requests,&requestsMutex)){
            memset(message, '\0', 512*sizeof(char));
            REQUEST *req = (REQUEST *)queueMutexPop(&requests, &requestsMutex);

            sprintf(message,"%f - %9.9d - %9.9d: %c - %9.9d - PEDIDO\n",getTime(), getpid(), req->serialNum, req->gender, req->time);
            write(fds.fileLog, message, sizeof(char)*512);

            write(fds.fifoRequests, req, sizeof(struct request_info));

            free(req);
        }
        if(endRejection){
            break;
        }
    }
}

void generateRequests(){
    int serialNum = 0;
    
    while (command.requests > 0) {
        REQUEST *req = malloc(sizeof(struct request_info));
        if(rand()%2 == 0)
            req->gender = 'F';
        else
            req->gender = 'M';
        req->serialNum = serialNum;
        req->rejections = 0;
        req->time = 1 + rand()%command.maxTime;
        
        queueMutexPush(req, &requests, &requestsMutex); //Sends this request to the bottom of the queueMutex

        serialNum++;
        command.requests--;
    }
}

void startRejectionHandler(pthread_t* tid){
    pthread_create(tid, NULL, rejectionHandler, NULL);
}

int main(int argc, char *argv[]){
    
    pthread_t tid;
    srand(time(NULL));
    requests.dynamic = 1;
    memset(&command,0,sizeof(struct command));

    getTime();

    argumentHandling(argc, argv);
    
    initCommunications();

    startRejectionHandler(&tid);

    generateRequests();

    sendRequests();

    pthread_join(tid, NULL);

    closeCommunications();
    
    queueMutexFree(&requests, &requestsMutex);
    return 0;
}
