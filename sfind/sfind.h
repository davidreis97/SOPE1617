#ifndef SFIND_H
#define SFIND_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wordexp.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>

pid_t father = 0;
pid_t child = 0;

typedef struct command{ //See default values in function below
    char *directory;
    char *name;
    char *type;
    int perm;
    int print;
    int delete;
    char **command;
}COMMAND; 

void initializeCommand(COMMAND *command){
    command->directory = NULL;
    command->name = NULL;
    command->type = NULL;
    command->perm = -1;
    command->print = 0;
    command->delete = 0;
    command->command = NULL;
}

#endif
