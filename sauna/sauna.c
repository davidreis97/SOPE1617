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
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/times.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>


typedef struct command{
    int slots;
}COMMAND;

COMMAND command;

FILEDESCRIPTORS fds;

clock_t start = 0;

pthread_mutex_t slotsMutex = PTHREAD_MUTEX_INITIALIZER;

int slotsAvailable;

QUEUE threads;

int *record = NULL;
int shmid = -1;
int semid = -1;

void semWait(){
    struct sembuf semopr;

    semopr.sem_num = 0;
    semopr.sem_op = -1;
    semopr.sem_flg = SEM_UNDO;

    if (semop(semid, &semopr, 1) == -1){
        perror("SEMOP (WAIT) ERROR");
        exit(EXIT_FAILURE);
    }
}

void semSignal(){
    struct sembuf semopr;

    semopr.sem_num = 0;
    semopr.sem_op = 1;
    semopr.sem_flg = SEM_UNDO;

    if (semop(semid, &semopr, 1) == -1){
        perror("SEMOP (SIGNAL) ERROR");
        exit(EXIT_FAILURE);
    }
}

double getTime() {
    clock_t end;
    struct tms t;
    long ticks;

    if(start == 0){
        start = times(&t);
    }

    ticks = sysconf(_SC_CLK_TCK);

    end = times(&t);

    return ((double)(end-start) * 1000/ticks);
}

void argumentHandling(int argc, char*argv[]){    
    if (argc != 2){
        printf("Usage: %s <slots>\n",argv[0]);
        exit(1);
    }

    command.slots = atoi(argv[1]);

    if(command.slots <= 0){
         fprintf(stderr, "Error! Number of requests must be a positive integer.\n");
    }

}

void initCommunications(){
	char filename[20];
    key_t key;
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

    key = ftok("gerador", 0);

    if((shmid = shmget(key, 0, SHM_R | SHM_W)) == -1){
        perror("SHMGET ERROR");
        exit(EXIT_FAILURE);
    }
    
    if((record = (int *) shmat(shmid, 0, 0)) == NULL){
        perror("SHMAT ERROR");
        exit(EXIT_FAILURE);
    }

    key = ftok("gerador", 1);

    if((semid = semget(key, 1, SEM_R | SEM_A)) == -1){
        perror("SEMGET ERROR");
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(fds.fifoRequests, F_GETFL, 0);
    fcntl(fds.fifoRequests, F_SETFL, flags | O_NONBLOCK);

}

void *processUser(void *arg){
    int *time = (int *)arg;

    sleep(*time);

    if(pthread_mutex_lock(&slotsMutex)){
        perror("MUTEX LOCK ERROR");
        exit(1);
    }

    slotsAvailable++;

    if(pthread_mutex_unlock(&slotsMutex)){
        perror("MUTEX LOCK ERROR");
        exit(1);
    }

    free(time);

    return NULL;
}

void startListener(){
    char currGender;
    REQUEST req;

    char message[512];
    memset(message,'\0',512*sizeof(char));

    slotsAvailable = command.slots;
    
    while(1){

        semWait();
        if(!record[REQUESTS] && !record[REJECTIONS]){
            semSignal();
            break;
        }
        semSignal();

        if(read(fds.fifoRequests,&req,sizeof(struct request_info)) > 0){

            memset(message,'\0',512*sizeof(char));
            sprintf(message,"%f - %9.9d - %9.9d - %9.9d: %c - %9.9d - RECEBIDO\n",
                getTime(), getpid(),0, req.serialNum, req.gender, req.time);
             
            write(fds.fileLog, message, sizeof(char)*512);

            if (currGender != req.gender && slotsAvailable != command.slots && slotsAvailable + 1 != command.slots){ //REJEITADO
                 
                memset(message,'\0',512*sizeof(char));
                sprintf(message,"%f - %9.9d - %9.9d - %9.9d: %c - %9.9d - REJEITADO\n",
                    getTime(), getpid(), 0, req.serialNum, req.gender, req.time);
                write(fds.fileLog, message, sizeof(char)*512);

                semWait();
                record[REJECTIONS]++;
                record[REQUESTS]--;
                semSignal();

                write(fds.fifoRejected,&req,sizeof(struct request_info));
             
            }else{ //ACEITE

                pthread_t *tid = malloc(sizeof(pthread_t));
                int *time = malloc(sizeof(int));
                
                //If the person is from the same gender, waits instead of rejecting. Also prevents a user from a certain gender from entering the sauna while there is one last member of the opposite gender inside.
                while(slotsAvailable <= 0 || (currGender != req.gender && slotsAvailable != command.slots));

                currGender = req.gender;

                semWait();
                record[REQUESTS]--;
                semSignal();

                if(pthread_mutex_lock(&slotsMutex)){
                    perror("MUTEX LOCK ERROR");
                    exit(1);
                }

                slotsAvailable--;

                if(pthread_mutex_unlock(&slotsMutex)){
                    perror("MUTEX LOCK ERROR");
                    exit(1);
                }

                *time = req.time;
                pthread_create(tid, NULL, processUser, time);
                 
                memset(message,'\0',512*sizeof(char));
                sprintf(message,"%f - %9.9d - %9.9d - %9.9d: %c - %9.9d - SERVIDO\n",
                    getTime(), getpid(),(unsigned int) *tid, req.serialNum, req.gender, req.time);

                write(fds.fileLog, message, sizeof(char)*512);

                queuePush(tid,&threads);
             }
        }
    }

    while(!queueIsEmpty(&threads)){
        pthread_t *tid = (pthread_t *) queuePop(&threads);
        pthread_join(*tid, NULL);
        free(tid);
    }
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

    if(shmdt(record)){
        perror("SHMDT ERROR");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[]){
    memset(&command,0,sizeof(struct command));

    threads.dynamic = 1;

    argumentHandling(argc, argv);

    initCommunications();

    startListener();

    closeCommunications();

    queueFree(&threads);

    return 0;
}
