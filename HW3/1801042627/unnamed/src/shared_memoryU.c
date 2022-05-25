#include "../header/sharad_memory.h"
int createSharedMemory(const char* programName,const char *name){
    int fd = 0;

    fd = shm_open(name, O_CREAT|/*O_EXCL|*/O_RDWR/*|FD_CLOEXEC*/, S_IRUSR | S_IWUSR);
    
    if(fd == -1){
        perror("shm_open");
        return -1;
    }
    return fd;
}
int removeSharedMemory(const char* programName,int sharedFd,const char* sharedMemoryName){
    if(close(sharedFd) == -1 || shm_unlink(sharedMemoryName) == -1){
        perror("Shared memory could not be closed");
        return -1;
    }
    return 0;
}
