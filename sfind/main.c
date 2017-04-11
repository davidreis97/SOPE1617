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

void keyboardHandler(int signo){
    char answer = '\0';
    do{
        printf("\nAre you sure you want to terminate (Y/N)? \n");
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
                printf("WARNING: -name is missing a value, argument ignored\n");
            }
        }else if(strcmp("-type",argv[argno]) == 0 && argno + 1 < argc){
            if(argv[argno+1][0] != '-'){
                command->type = argv[argno+1];
                argno++;
            }else{
                printf("WARNING: -type is missing a value, argument ignored\n");
            }
        }else if(strcmp("-perm",argv[argno]) == 0 && argno + 1 < argc){
            if(argv[argno+1][0] != '-'){
                command->perm = atoi(argv[argno+1]);
                argno++;
            }else{
                printf("WARNING: -perm is missing a value, argument ignored\n");
            }
        }else if(strcmp("-print",argv[argno]) == 0){
            command->print = 1;
        }else if(strcmp("-delete",argv[argno]) == 0){
            char answer = '\0';
            do{
                printf("WARNING: The -delete option will PERMANENTLY DELETE all files found RECURSIVELY from the directory [%s].\nDo you wish to procede? (Y/N)\n",command->directory);
                scanf(" %c",&answer);
                if(answer == 'N' || answer == 'n'){
                    printf("Terminating.\n");
                    exit(0);
                }
            }while(answer != 'Y' && answer != 'y');
            command->delete = 1;
        }else if(strcmp("-exec",argv[argno]) == 0 && argno + 1 < argc){
            command->command = &argv[argno+1];
            argno++;
        }
    }
}

void argumentValidation(COMMAND *command, wordexp_t *expansion){
    //Processing tilde ('~') on directory
    if (wordexp(command->directory, expansion, 0) != 0){
        perror("WORDEXP error");
    }
    command->directory = expansion->we_wordv[0];

    //Remove '/' do fim do path. Ex: /Users/davidreis/ -> /Users/davidreis
    if(command->directory[strlen(command->directory)-1] == '/'){ 
        command->directory[strlen(command->directory)-1] = '\0';
    }

    //Check if type is valid
    if(command->type != NULL){
        if((command->type[0] != 'f' && command->type[0] != 'd' && command->type[0] != 'l') || strlen(command->type) != 1){
            fprintf(stderr,"Type %s is not valid!\n[f - file | d - directory | l - symlink]\n",command->type);
            exit(1);
        }
    }
}

void cleanup(wordexp_t *expansion){
    wordfree(expansion);
}

void printFileInfo(char *filepath, struct stat dir_info){
    if(S_ISREG(dir_info.st_mode)){
        printf("%s 'regular'\n",filepath); 
    }else if(S_ISLNK(dir_info.st_mode)){
        printf("%s 'symlink'\n",filepath);
    }else if(S_ISDIR(dir_info.st_mode)){
        printf("%s 'directory'\n",filepath);
    }else{
        printf("%s 'unknown'\n",filepath);
    }
}

int traverseDirectory(COMMAND command);

int processNewDirectory(char *filepath, COMMAND command){
    pid_t childPID;

    childPID = fork();
           
    if(childPID == 0){ //FILHO
        int return_status = 0;

        command.directory = filepath;
        return_status = traverseDirectory(command);

        exit(return_status);
    }else{ //PAI E ERRO (PAI TRATA DO ERRO)
        int status = 0;

        waitpid(childPID, &status, 0);
        
        return WEXITSTATUS(status);
    }
}

int convertDecimalToOctal(int decimalNumber) {
    int octalNumber = 0, i = 1;

    while (decimalNumber != 0)
    {
        octalNumber += (decimalNumber % 8) * i;
        decimalNumber /= 8;
        i *= 10;
    }

    return octalNumber;
}

int correctPerm(struct stat dir_info, COMMAND command){
    if(command.perm == -1){
        return 1;
    }

    int filePerm = dir_info.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    
    filePerm = convertDecimalToOctal(filePerm);

    if (command.perm == filePerm){
        return 1;
    }else{
        return 0;
    }
}

int correctType(struct stat dir_info, COMMAND command){
    if(command.type == NULL){
        return 1;
    }

    if(*command.type == 'f' && !S_ISREG(dir_info.st_mode)){
        return 0;
    }

    if(*command.type == 'd' && !S_ISDIR(dir_info.st_mode)){
        return 0;
    }

    if(*command.type == 'l' && !S_ISLNK(dir_info.st_mode)){
        return 0;
    }

    return 1;
}

int correctName(char *filename, COMMAND command){
    if(command.name == NULL){
        return 1;
    }

    if(strcmp(command.name,filename)){
        return 0;
    }

    return 1;
}

int executeCommand(char *filepath, COMMAND command){ //TODO -> "-exec"
    printf("Executing...\n");
    return 0;
}

int traverseDirectory(COMMAND command){
    DIR* dir;
    struct dirent *dp;
    struct stat dir_info;

    dir = opendir(command.directory);

    if(dir == NULL){
        perror("OPENDIR ERROR");
        return -1;
    }

    while((dp = readdir(dir)) != NULL){
        char *filepath;

        filepath = malloc((strlen(command.directory) + strlen(dp->d_name) + 2) * sizeof(char)); //command.directory + '/' + dp->d_name + '\0'
        strcpy(filepath,command.directory);
        strcat(filepath,"/");
        strcat(filepath,dp->d_name);

        if(lstat(filepath, &dir_info) < 0){
            perror("STAT ERROR");
            fprintf(stderr,"Error in: %s \n",filepath);
            return -1;
        }

        if(!strcmp(dp->d_name, "..") || !strcmp(dp->d_name, ".")){
            continue;
        }

        if(correctType(dir_info,command) && correctName(dp->d_name,command) && correctPerm(dir_info, command)){
            if(command.command != NULL){
                int commandReturn;
                if ((commandReturn = executeCommand(filepath, command)) != 0){
                    fprintf(stderr,"Error executing the -exec.\n");
                    return commandReturn;
                }
            }
            if(command.print){
                 printFileInfo(filepath,dir_info);
            }
            if(command.delete && !S_ISDIR(dir_info.st_mode)){
                char answer = '\0';
                do{
                    printf("WARNING: This will delete the file [%s].\nDo you wish to procede? (Y/N)\n",filepath);
                    scanf(" %c",&answer);
                    if(answer == 'Y' || answer == 'y'){
                        if (unlink(filepath) != 0){
                            perror("UNLINK ERROR");
                            return -1;
                        }
                    }
                }while(answer != 'N' && answer != 'n' && answer != 'Y' && answer != 'y');
            }
        }

        if(S_ISDIR(dir_info.st_mode)){
            if(processNewDirectory(filepath, command) != 0){
                perror("ERROR FORKING");
                return -1;
            }
        }

        free(filepath);
    }
    
    if (closedir(dir) != 0){
        perror("CLOSEDIR ERROR");
        return -1;
    }  

    return 0;
}

int main(int argc, char *argv[]){
    wordexp_t expansion;
    COMMAND command;
    pid_t initial;
    int status = 0;

    initializeCommand(&command);

    argumentHandling(argc, argv, &command);

    argumentValidation(&command, &expansion);

    initial = fork();

    if(initial > 0){ //Pai
        signalProcessing();
        waitpid(initial,&status,0);
    }else if(initial == 0){ //Filho
        return traverseDirectory(command);
    }else{ //Erro
        perror("ERROR FORKING");
    }

    cleanup(&expansion);

    return WEXITSTATUS(status);
}
