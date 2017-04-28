#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct command{
    int requests;
    int maxTime;
    char timeUnit;
}COMMAND;

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


int main(int argc, char *argv[]){
    COMMAND command;
    memset(&command,0,sizeof(struct command));

    argumentHandling(argc, argv, &command);

}
