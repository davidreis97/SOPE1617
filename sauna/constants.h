#define OP_MODE 0777
#define WRITE 1
#define READ 0
#define REJECTIONS 1
#define REQUESTS 0

typedef struct request_info{
    int serialNum;
    char gender;
    int time;
    int rejections;
} REQUEST;

typedef struct fileDescriptors{
    int fifoRequests;
    int fifoRejected;
    int fileLog;
} FILEDESCRIPTORS;
