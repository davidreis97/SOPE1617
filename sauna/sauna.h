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
#include <time.h>

typedef struct command{
    int slots;
}COMMAND;

//Performs a wait action on the semaphore with id in global variable "semid".
void semWait();

//Performs a signal action on the semaphore with id in global variable "semid".
void semSignal();

//Returns the time in milliseconds since the start of the program. First value returned is garbage (0).
double getTime();

//Processes arguments received from the command line.
void argumentHandling(int argc, char*argv[]);

//Initializes/opens all communication elements (FIFO, log files, semaphores, etc).
void initCommunications();

//Closes all communication elements (FIFO, log files, semaphores, etc).
void closeCommunications();

//Represents a user in the sauna. Sleeps for the amount of time specified by argument. Updates slotsAvailable variable.
void *processUser(void *arg);

//Reads requests from requests FIFO and chooses between rejecting them or accepting them into the sauna.
void startListener();
