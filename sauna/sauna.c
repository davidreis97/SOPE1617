#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct command{
    int slots;
    char timeUnit;
}COMMAND;

/*typedef struct fifos_fds{
	int requests;
	int rejected;		//fifos_fds perhaps is not the best option, maybe creating a struct with the two 
} FIFO;*/				// fifos fds is a beeter option? 	

int DEBUG = 0;

void argumentHandling(int argc, char*argv[], COMMAND *command){    
    if (argc != 3){
        printf("Usage: %s <slots> <time unit>\n",argv[0]);
        exit(1);
    }

    command->slots = atoi(argv[1]);

    if(command->slots <= 0){
         fprintf(stderr, "Error! Number of requests must be a positive integer.\n");
    }

    command->timeUnit = argv[2][0];

    if (strlen(argv[2]) != 1 || (argv[2][0] != 's' && argv[2][0] != 'm' && argv[2][0] != 'u')){
        fprintf(stderr, "Error! '%s' is not a valid time unit.\n[s- seconds / m- milliseconds / u- microseconds]\n",argv[2]);
    }
}

void initCommunications(int * fifos_fds){

	if((fifos_fds[0] = open("/tmp/entrada", O_RDONLY))< 0){
		perror("Couldn't open FIFO '/tmp/entrada' ");
		exit(EXIT_FAILURE);
	}

	if((fifos_fds[1] = open("/tmp/rejeitados", O_WRONLY)) < 0){
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
}


int main(int argc, char *argv[]){
	int fifos_fds[2];
    COMMAND command;

    memset(&command,0,sizeof(struct command));

    argumentHandling(argc, argv, &command);

    initCommunications(fifos_fds);

    closeCommunications(fifos_fds);

}
