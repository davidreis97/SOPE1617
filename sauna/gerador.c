#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define OP_MODE 0777

typedef struct command{
    int requests;
    int maxTime;
    char timeUnit;
} COMMAND;

typedef struct request_info{
    int serial_num;
    char gender;
    float time;
} REQUEST;

typedef struct fileDescriptors{
    int fifo_requests;
    int fifo_rejected;
    int file_info;
} fileDescriptors;

int requestsProcessed = 0;

REQUEST* requestsQueue;

int DEBUG = 0;

void argumentHandling(int argc, char*argv[], COMMAND *command){    
    if (argc != 4){
        printf("Usage: %s <requests> <max. time> <time unit>\n",argv[0]);
        exit(1);
    }

    command->requests = atoi(argv[1]);

    if(command->requests <= 0){
         fprintf(stderr, "Error! Number of requests must be a positive integer.\n");
    }

    command->maxTime = atoi(argv[2]);

    if(command->maxTime <= 0){
         fprintf(stderr, "Error! Maximum time must be a positive integer.\n");
    }

    command->timeUnit = argv[3][0];

    if (strlen(argv[3]) != 1 || (argv[3][0] != 's' && argv[3][0] != 'm' && argv[3][0] != 'u')){
        fprintf(stderr, "Error! '%s' is not a valid time unit.\n[s- seconds / m- milliseconds / u- microseconds]\n",argv[3]);
    }
}

void initCommunications(fileDescriptors* fds){

    char filename[20];
    
    sprintf(filename, "/tmp/ger.%d", getpid());

    if((fds->file_info = open(filename, O_WRONLY | O_CREAT | O_EXCL, OP_MODE)) < 0){
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


    if((fds->fifo_requests = open("/tmp/entrada", O_WRONLY)) < 0){
        perror("Couldn't open FIFO '/tmp/entrada' ");
        exit(EXIT_FAILURE); 
    }

    if((fds->fifo_rejected = open("/tmp/rejeitados", O_RDONLY)) < 0){
        perror("Couldn't open FIFO '/tmp/rejeitados' ");
        exit(EXIT_FAILURE);
    }

}

void closeCommunications(fileDescriptors* fds){

    if(close(fds->file_info) < 0){
        perror("Couldn't close file ");
        exit(EXIT_FAILURE);
    }

    if(close(fds->fifo_requests) < 0){
        perror("Couldn't close '/tmp/entrada' ");
        exit(EXIT_FAILURE);
    }

    if(close(fds->fifo_rejected) < 0){
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

void runCommunications(fileDescriptors* fds, COMMAND* command){

    /*while(requestsProcessed < command->request){

    }*/
}

int main(int argc, char *argv[]){

    fileDescriptors fds;

    COMMAND command;
    memset(&command,0,sizeof(struct command));

    argumentHandling(argc, argv, &command);
    
    initCommunications(&fds);

    runCommunications(&fds, &command);

    printf("%d", fds.file_info);

    closeCommunications(&fds);

    return 0;
}
