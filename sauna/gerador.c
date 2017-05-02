#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "queue.h"

#define WRITE 1
#define READ 0

typedef struct command{
    int requests;
    int maxTime;
    char timeUnit;
}COMMAND;

COMMAND command;

QUEUE queue;

int DEBUG = 0;

void argumentHandling(int argc, char*argv[]){    
    if (argc != 4){
        printf("Usage: %s <requests> <max. time> <time unit>\n",argv[0]);
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

    command.timeUnit = argv[3][0];

    if (strlen(argv[3]) != 1 || (argv[3][0] != 's' && argv[3][0] != 'm' && argv[3][0] != 'u')){
        fprintf(stderr, "Error! '%s' is not a valid time unit.\n[s- seconds / m- milliseconds / u- microseconds]\n",argv[3]);
    }
}

void queue_mutex_push(int value){
    //lock
    queue_push(value, &queue);
    //unlock
}

int queue_mutex_pop(){
    int temp;
    //lock
    temp = queue_pop(&queue);
    //unlock
    return temp;
}

int main(int argc, char *argv[]){
    memset(&command,0,sizeof(struct command));

    argumentHandling(argc, argv);

    //startGenerationThread();

    //startListener();
}
