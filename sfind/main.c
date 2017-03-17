#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

typedef struct command{
    char *directory;
    char *name;
    char *type;
    int perm;
    int print;
    int delete;
    char **command;
}COMMAND;

int DEBUG = 0;

void keyboardHandler(int signo){
    char answer = '\0';
    do{
        printf("Are you sure you want to terminate (Y/N)? \n");
        scanf(" %c",&answer);
        if(answer == 'Y' || answer == 'y'){
            printf("Exiting through SIGINT\n");
            exit(0);
        }
    }while(answer != 'N' && answer != 'n');
}

void signalProcessing(){
    struct sigaction act;

    act.sa_handler = keyboardHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    
    if(sigaction(SIGINT,&act,NULL) != 0){
        perror("SIGACTION ERROR");
    }
}

void argumentHandling(int argc, char*argv[], COMMAND *command){
    int argno = 0;
    
    if (argc < 2){
        printf("Invalid no of arguments\n");
        exit(1);
    }

    command->directory = argv[1];

    for(argno = 2; argno < argc; argno++){
        if(strcmp("-name",argv[argno]) == 0 && argno + 1 < argc){
            if(argv[argno+1][0] != '-'){
                command->name = argv[argno+1];
                argno++;
            }else{
                printf("-name is missing a value, argument ignored\n");
            }
        }else if(strcmp("-type",argv[argno]) == 0 && argno + 1 < argc){
            if(argv[argno+1][0] != '-'){
                command->type = argv[argno+1];
                argno++;
            }else{
                printf("-type is missing a value, argument ignored\n");
            }
        }else if(strcmp("-perm",argv[argno]) == 0 && argno + 1 < argc){
            if(argv[argno+1][0] != '-'){
                command->perm = atoi(argv[argno+1]);
                argno++;
            }else{
                printf("-perm is missing a value, argument ignored\n");
            }
        }else if(strcmp("-print",argv[argno]) == 0){
            command->print = 1;
        }else if(strcmp("-delete",argv[argno]) == 0){
            command->delete = 1;
        }else if(strcmp("-exec",argv[argno]) == 0 && argno + 1 < argc){
            command->command = &argv[argno+1];
            argno++;
        }else if(strcmp("-debug",argv[argno]) == 0){
            DEBUG = 1;
        }
    }
}


int main(int argc, char *argv[]){
    COMMAND command;
    memset(&command,0,sizeof(struct command));

    argumentHandling(argc, argv, &command);

    signalProcessing();

    //processFiles();
}