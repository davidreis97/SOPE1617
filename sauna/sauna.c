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

#define OP_MODE 0777

typedef struct command{
    int slots;
    char timeUnit;
}COMMAND;

int DEBUG = 0;

COMMAND command;

FILEDESCRIPTORS fds;

QUEUE threads;

void argumentHandling(int argc, char*argv[]){    
    if (argc != 3){
        printf("Usage: %s <slots> <time unit>\n",argv[0]);
        exit(1);
    }

    command.slots = atoi(argv[1]);

    if(command.slots <= 0){
         fprintf(stderr, "Error! Number of requests must be a positive integer.\n");
    }

    command.timeUnit = argv[2][0];

    if (strlen(argv[2]) != 1 || (argv[2][0] != 's' && argv[2][0] != 'm' && argv[2][0] != 'u')){
        fprintf(stderr, "Error! '%s' is not a valid time unit.\n[s- seconds / m- milliseconds / u- microseconds]\n",argv[2]);
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

void startListener(){

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

    threads.dynamic = 0; //Pode ter de ser alterado.

    argumentHandling(argc, argv);

    initCommunications();

    startListener();

    closeCommunications();

}
