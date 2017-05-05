#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "constants.h"
#include "queue.h"

typedef struct command{
    int slots;
}COMMAND;

int DEBUG = 0;

COMMAND command;

FILEDESCRIPTORS fds;

pthread_mutex_t slotsMutex = PTHREAD_MUTEX_INITIALIZER;

int slotsAvailable;

QUEUE threads;

void argumentHandling(int argc, char*argv[]){    
    if (argc != 2){
        printf("Usage: %s <slots>\n",argv[0]);
        exit(1);
    }

    command.slots = atoi(argv[1]);

    if(command.slots <= 0){
         fprintf(stderr, "Error! Number of requests must be a positive integer.\n");
    }

}

void initCommunications(){

	char filename[20];
    sprintf(filename, "/tmp/bal.%d", getpid());

    if((fds.fileLog = open(filename, O_WRONLY | O_CREAT | O_EXCL, OP_MODE)) < 0){
    	perror("Couldn't create fileInfo ");
        exit(EXIT_FAILURE);
    }

	if((fds.fifoRequests = open("/tmp/entrada", O_RDONLY))< 0){
		perror("Couldn't open FIFO '/tmp/entrada' ");
		exit(EXIT_FAILURE);
	}

	if((fds.fifoRejected = open("/tmp/rejeitados", O_WRONLY)) < 0){
        perror("Couldn't open FIFO '/tmp/rejeitados' ");
        exit(EXIT_FAILURE);
    }


}

void *processUser(void *arg){
    int *time = (int *)arg;

    printf("%d\n",*time);
    sleep(*time);

    if(pthread_mutex_lock(&slotsMutex)){
        perror("MUTEX LOCK ERROR");
        exit(1);
    }

    slotsAvailable++;

    if(pthread_mutex_unlock(&slotsMutex)){
        perror("MUTEX LOCK ERROR");
        exit(1);
    }

    free(time);

    return NULL;
}

void startListener(){
    char currGender;
    REQUEST req;

    slotsAvailable = command.slots;
    
    while(read(fds.fifoRequests,&req,sizeof(struct request_info))){        
        if (currGender != req.gender && slotsAvailable != command.slots){
            //write(fds.fifoRejected,&req,sizeof(struct request_info));
        }else{
            currGender = req.gender;
            pthread_t *tid = malloc(sizeof(pthread_t));
            int *time = malloc(sizeof(int));

            printf("REQUEST: %c / %d\n",req.gender,req.time);
            
            while(slotsAvailable <= 0);

            if(pthread_mutex_lock(&slotsMutex)){
                perror("MUTEX LOCK ERROR");
                exit(1);
            }

            slotsAvailable--;

            if(pthread_mutex_unlock(&slotsMutex)){
                perror("MUTEX LOCK ERROR");
                exit(1);
            }

            *time = req.time;
            pthread_create(tid, NULL, processUser, time);

            queuePush(tid,&threads);
        }
    }

    while(!queueIsEmpty(&threads)){
        pthread_t *tid = (pthread_t *) queuePop(&threads);
        pthread_join(*tid, NULL);
        free(tid);
    }
}

void closeCommunications(){

	if(close(fds.fileLog) < 0){
		perror("Couldn't close file_info ");
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
}


int main(int argc, char *argv[]){
    memset(&command,0,sizeof(struct command));

    threads.dynamic = 1;

    argumentHandling(argc, argv);

    initCommunications();

    startListener();

    closeCommunications();

    queueFree(&threads);
}
