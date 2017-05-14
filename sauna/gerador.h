#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "queue.h"
#include "constants.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/times.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

typedef struct command{
    int requests;
    int maxTime;
} COMMAND;

//Intended to run atexit of program, deletes semaphore and shared memory block.
void sharedCleaner(void);

//Performs a wait action on the semaphore with id in global variable "semid".
void semWait();

//Performs a signal action on the semaphore with id in global variable "semid".
void semSignal();

//Returns the time in milliseconds since the start of the program. First value returned is garbage (0).
double getTime();

//Reads requests from the rejections FIFO, chooses between discarding the request or pushing it back on the queue.
void* rejectionHandler(void* arg);

//Processes arguments received from the command line.
void argumentHandling(int argc, char*argv[]);

//Initializes/opens all communication elements (FIFO, log files, semaphores, etc).
void initCommunications();

//Closes all communication elements (FIFO, log files, etc) except the ones closed by sharedCleaner().
void closeCommunications();

//Pops from the queue and writes all the requests to the requests FIFO.
void sendRequests();

//Generates all the initial requests and pushes them to the queue.
void generateRequests();

//Starts the rejection handler thread and stores it's TID to the pointer passed by argument.
void startRejectionHandler(pthread_t* tid);
