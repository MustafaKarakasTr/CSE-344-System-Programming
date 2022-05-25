#include "../header/create_semaphore.h"

int create_sem(sem_t** sem,char isItNamed,char* sharedName,int flag,int initialVal){
    if(!isItNamed){
        perror("QQQQQQQQQQQQqqq");
       *sem = sem_open(sharedName,flag, S_IRUSR | S_IWUSR,initialVal);
        if(*sem == SEM_FAILED){
            perror("sem_open");
            return -1;
        }
    } else {
        sem_init(*sem,0,initialVal);
        perror("IT IS UNNAMED");
    }
    return 0;
}