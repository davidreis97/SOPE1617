#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct command{
    int slots;
    char timeUnit;
}COMMAND;

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


int main(int argc, char *argv[]){
    COMMAND command;
    memset(&command,0,sizeof(struct command));

    argumentHandling(argc, argv, &command);

}
