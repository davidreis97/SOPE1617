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
} INFO;

/*typedef struct fifos_fds{
    int requests;

}*/

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

void initCommunications(int * fifos_fds){

    /* Creates FIFO's "entrada" e "rejeitados" */

    if(mkfifo("/tmp/entrada", OP_MODE) < 0){
        perror("Couldn't create FIFO '/tmp/entrada' ");
        exit(EXIT_FAILURE);
    }

    if(mkfifo("/tmp/rejeitados", OP_MODE) < 0){
        perror("Couldn't create FIFO '/tmp/rejeitados' ");
        exit(EXIT_FAILURE);
    }


    if((fifos_fds[0] = open("/tmp/entrada", O_WRONLY)) < 0){
        perror("Couldn't open FIFO '/tmp/entrada' ");
        exit(EXIT_FAILURE); 
    }

    if((fifos_fds[1] = open("/tmp/rejeitados", O_RDONLY)) < 0){
        perror("Couldn't open FIFO '/tmp/rejeitados' ");
        exit(EXIT_FAILURE);
    }
}

void closeCommunications(int * fifos_fds){

    if(close(fifos_fds[0]) < 0){
        perror("Couldn't close '/tmp/entrada' ");
        exit(EXIT_FAILURE);
    }

    if(close(fifos_fds[1]) < 0){
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

int main(int argc, char *argv[]){
    int fifos_fds[2];
    COMMAND command;
    memset(&command,0,sizeof(struct command));

    argumentHandling(argc, argv, &command);
    
    initCommunications(fifos_fds);

    printf("%d", fifos_fds[0]);

    closeCommunications(fifos_fds);

    return 0;
}
