#include<stdlib.h>
#include<fcntl.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<signal.h>
#include<fcntl.h>
#include <locale.h>
#include <time.h>
#include <sys/time.h>
#include "semun.h"
#define BUFFERSIZE 150
//./hw4 -C 10 -N 5 -F inputfilePath
sig_atomic_t terminate = 0;
int semaphorePass = 0;
struct timeval tv;
int semid;
int N;
int removeSem(int semId){
    if (semctl(semId, 0, IPC_RMID) == -1) // remove the set
    {    
        perror("semctl-IPC_RMID");
        return -1;
    }
    return 0;
}

void signalHandler(int sig){
    terminate = 1;
    
}
void * consumerFunc(void* idOfConsumers){
    struct timeval now;

    union semun dummy;
    unsigned short valuesDummy[2];
    dummy.array = valuesDummy;
    //int * args = *(int*)idOfConsumers;

    int idOfconsumer = *(int*)idOfConsumers;
    struct sembuf sops[2];
    sops[0].sem_num = 0;
    sops[0].sem_op = -2;

    sops[1].sem_num = 1;
    sops[1].sem_op = -2;
    struct flock lock;
    char buf[BUFFERSIZE];
    char formatWaiting[] = "seconds: %ld microseconds: %ld Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2’.\n";
    char formatConsumed[] = "seconds: %ld microseconds: %ld Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x ‘1’, %d x ‘2’.\n";
    char formatLeft[] = "seconds: %ld microseconds: %ld Consumer-%d has left.\n";

    for (int i = 0; i < N; i++)
    {
        if (semctl(semid, 0, GETALL, dummy) == -1)
        {
            perror("semctl getall consumer");
            return NULL;
        }
        if (gettimeofday(&now, NULL) == -1){
            perror("gettimeofday");
            break;
        }
        long secondsPassed = (long) now.tv_sec - tv.tv_sec;
        long microsecondsPassed = (long) now.tv_usec - tv.tv_usec;
        // make output ready before locking the file
        if(snprintf(buf,BUFFERSIZE,formatWaiting,secondsPassed,microsecondsPassed,idOfconsumer,i,dummy.array[0]/2,dummy.array[1]/2) < 0){
            perror("snprintf consumed");
            return NULL;
        }
        if(terminate){//perror("AA61");
            return 0;
        }
        // lock file
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        if(terminate){//perror("AA2");
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
            return 0;
        }
        
        while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
        //fprintf(stdout,"Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2’.\n",idOfconsumer,i,dummy.array[0]/2,dummy.array[1]/2);
        lock.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
        if(terminate){
            return 0;
        }

        //fprintf("%d ",idOfconsumer);
        if(semop(semid,sops,2) == -1){
            perror("semop");
            return NULL;
        }
        if(terminate){
            return 0;
        }
        if (semctl(semid, 0, GETALL, dummy) == -1)
        {
            perror("semctl getall");
            return NULL;
        } 
        if (gettimeofday(&now, NULL) == -1){
            perror("gettimeofday");
            break;
        }
        secondsPassed = (long) now.tv_sec - tv.tv_sec;
        microsecondsPassed = (long) now.tv_usec - tv.tv_usec;
        // make output ready before locking the file
        if(snprintf(buf,BUFFERSIZE,formatConsumed,secondsPassed,microsecondsPassed,idOfconsumer,i,dummy.array[0]/2,dummy.array[1]/2) < 0){
            perror("snprintf consumed");
            return NULL;
        }
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        if(terminate){
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
            return 0;
        }
        while((write(STDOUT_FILENO,buf,strlen(buf)) == -1 ) && errno == EINTR); 

        //fprintf(stdout,"Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x ‘1’, %d x ‘2’.\n",idOfconsumer,i,dummy.array[0]/2,dummy.array[1]/2);
        lock.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
        if(terminate){//perror("AA8");
            return NULL;
        }
        //sleep(3);
        if(terminate){//perror("AA11");
            //pthread_exit(0);
            return 0;
        }
    }
    if (gettimeofday(&now, NULL) == -1){
        perror("gettimeofday");
        return NULL;
    }
    long secondsPassed = (long) now.tv_sec - tv.tv_sec;
    long microsecondsPassed = (long) now.tv_usec - tv.tv_usec;
    if(snprintf(buf,BUFFERSIZE,formatLeft,secondsPassed,microsecondsPassed,idOfconsumer) < 0){
        perror("snprintf consumer left");
        return NULL;
    }
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1 ) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
    
    return 0;
    
}
            
int incrementSemVal(int semid,int index){
    struct sembuf sops;
    sops.sem_num = index;
    sops.sem_op = 1;

    if(semop(semid,&sops,1) == -1){
        perror("semop");
        return -1;
    }
    return 0;
}
int createSemaphores(){
    union semun arg;
    unsigned short values[2] = {0,0};
    arg.array = values;
    int nsems = 2;
    int semid = semget(IPC_PRIVATE, nsems, S_IRUSR | S_IWUSR); // IPC_CREAT | IPC_EXCL
    if(semid == -1){
        perror("semget");
        return -1;    
    }
    int IGNORED = 0;
    if (semctl (semid, IGNORED, SETALL, arg) == -1)
    {
        perror("semctl");
        return -1;
    }
    return semid;
}
     
pthread_t* createThreads(int numberOfThreads,int* idOfConsumers){
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)* numberOfThreads);
    if(threads == NULL){
        return NULL;
    }

    for (int i = 0; i < numberOfThreads; i++)
    {
        if(pthread_create(&threads[i],NULL,consumerFunc,&idOfConsumers[i]) != 0){
            perror("pthread_create");
            // wait for threads created so far
            for (int j = 0; j < i; j++)
            {
                pthread_join(threads[j],NULL);
            }
            free(threads);
            return NULL;
        }
    }
    return threads;
}
/*
if(snprintf(buf,BUFFERSIZE,formatLeft,idOfconsumer) < 0){
    perror("snprintf consumer left");
    return -1;
}
memset(&lock,0,sizeof(lock));
lock.l_type = F_WRLCK;
fcntl(STDOUT_FILENO,F_SETLKW,&lock);
while((write(STDOUT_FILENO,buf,strlen(buf)) == -1 ) && errno == EINTR);
lock.l_type = F_UNLCK;
fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
*/
int removeSemLock(){
    union semun arg;
    unsigned short values[2] = {semaphorePass,semaphorePass};
    arg.array = values;
    if (semctl (semid, 0, SETALL, arg) == -1)
    {
        perror("semctl");
        return -1;
    }
    return 0;
}
void * producerFunc(void* argvVoid){
    //setbuf(stdout,NULL);
    struct timeval now;
    char buf[BUFFERSIZE];
    char formatRead[] = "seconds: %ld microseconds: %ld Supplier: read from input a ‘%c’. Current amounts: %d x ‘1’, %d x ‘2’.\n";
    char formatDelivered[] = "seconds: %ld microseconds: %ld Supplier: delivered a ‘%c’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n";
    char formatLeft[] = "seconds: %ld microseconds: %ld The Supplier has left\n";
    char * inputFile = ((char**)argvVoid)[6];
    //fprintf(stdout,"%d %d",C,N);
    int bytesread;
    int fd = open(inputFile,O_RDONLY);
    if (fd == -1){
        perror("open");
        //free(threads);
        //removeSem(semid);
        //free(idOfConsumers);
        ////free(args);
        return NULL;
    }
    union semun dummy;
    unsigned short valuesDummy[2];
    dummy.array = valuesDummy;
    char c;
    struct flock lock;
    long secondsPassed;
    long microsecondsPassed;
    while(1){
        //sleep(2);
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_RDLCK;
        fcntl(fd,F_SETLKW,&lock);
        while((bytesread = read(fd,&c,1)) == -1 && errno == EINTR);
        lock.l_type = F_UNLCK;
        fcntl(fd,F_SETLKW,&lock);
        if(bytesread != 1 || ( c!= '1' && c!= '2' ) ){
            if (gettimeofday(&now, NULL) == -1){
                perror("gettimeofday");
                break;
            }
            long secondsPassed = (long) now.tv_sec - tv.tv_sec;
            long microsecondsPassed = (long) now.tv_usec - tv.tv_usec;
            if(snprintf(buf,BUFFERSIZE,formatLeft,secondsPassed,microsecondsPassed) < 0){
                perror("snprintf producer read");
                break;
            }
            memset(&lock,0,sizeof(lock));
            lock.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);

            while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
            //fprintf(stdout,"The Supplier has left\n");
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
            break;
        }
        if (semctl(semid, 0, GETALL, dummy) == -1 )
        {    
            perror("semctl-GETALL");    
            /*free(threads);
            free(idOfConsumers);
            ////free(args);
            removeSem(semid);
            close(fd);
            */
            break;
            //return -1;
        }
        if(terminate){//perror("AAb");
            removeSemLock();
            break;
        }
        
        if (gettimeofday(&now, NULL) == -1){
            perror("gettimeofday");
            break;
        }
        secondsPassed = (long) now.tv_sec - tv.tv_sec;
        microsecondsPassed = (long) now.tv_usec - tv.tv_usec;
        
        if(snprintf(buf,BUFFERSIZE,formatRead,secondsPassed,microsecondsPassed,c,dummy.array[0]/2,dummy.array[1]/2) < 0){
            perror("snprintf producer read");
            break;
        }
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        if(terminate){//perror("AAc");
            removeSemLock();
            break;
        }
        //fprintf(stdout,"Supplier: read from input a ‘%c’. Current amounts: %d x ‘1’, %d x ‘2’.\n",c,dummy.array[0]/2,dummy.array[1]/2);
        while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
        lock.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
        if(terminate){//perror("AAe");
            removeSemLock();
            break;
        }
        int indexOfSem = c-'1';
        if(incrementSemVal(semid,indexOfSem) == -1){
            break;
        }
        /*if(c == '1' || c == '2'){
            if(incrementSemVal(semid,indexOfSem) == -1){
                break;
            }
        } else{
            perror("Invalid file content");
           
            break;
        }*/
        if (semctl(semid, 0, GETALL, dummy) == -1)
        {    
            perror("semctl-GETALL");
            // free(threads);
            // removeSem(semid);
            // free(idOfConsumers);
            // //free(args);
            // close(fd);
            // return -1;
            break;
        }
        if(terminate){//perror("AAt");
            removeSemLock();
            break;
        }
        if (gettimeofday(&now, NULL) == -1){
            perror("gettimeofday");
            break;
        }
        secondsPassed = (long) now.tv_sec - tv.tv_sec;
        microsecondsPassed = (long) now.tv_usec - tv.tv_usec;
        if(snprintf(buf,BUFFERSIZE,formatDelivered,secondsPassed,microsecondsPassed,c,(dummy.array[0]+1)/2,(dummy.array[1]+1)/2) < 0){
            perror("snprintf producer delivered");
            break;
        }
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        if(terminate){//perror("AAq");
            removeSemLock();
            break;
        }
        while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
        //fprintf(stdout,"Supplier: delivered a ‘%c’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",c,(dummy.array[0]+1)/2,(dummy.array[1]+1)/2);
        lock.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock); 
        if(incrementSemVal(semid,indexOfSem) == -1){
            break;
        }
    }

    close(fd);
    return 0;
}
int main(int argc, char const *argv[])
{
    if(argc != 7){
        perror("Invalid arguments, Valid run command: ./hw4 -C 10 -N 5 -F inputFilePath");
        return -1;
    }
    if (gettimeofday(&tv, NULL) == -1){
        perror("gettimeofday");
        return -1;
    }
    setbuf(stdout,NULL);
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &signalHandler;
    if ((sigemptyset(&sa.sa_mask) == -1) ||  (sigaction(SIGINT,&sa, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    // create detached producer thread
    pthread_t producer;
    pthread_attr_t detachedThread;

    if(pthread_attr_init(&detachedThread) != 0){
        perror("pthread_attr_init");
        return -1;
    }
    if(pthread_attr_setdetachstate(&detachedThread,PTHREAD_CREATE_DETACHED) != 0){
        perror("pthread_attr_setdetachstate");
        return -1;
    }
    if(pthread_create(&producer,&detachedThread,producerFunc,argv) != 0){
        perror("pthread_create");
        return -1;
    }
    // take input from command arguments
    int C = atoi(argv[2]);
    if(C <= 4){
        perror("C should be greater than 4");
        return -1;
    }
    semaphorePass = 5 * C; // make all consumer continue to terminate them properly
    N = atoi(argv[4]);
    if(N <= 1){
        perror("N should be greater than 1");
        return -1;
    }
    semid = createSemaphores();
    if(semid == -1){
        perror("Semaphores could not be created");
        pthread_attr_destroy(&detachedThread);
        pthread_exit(0);
    }

    int * idOfConsumers = (int*) malloc(sizeof(int) * C);
    if(idOfConsumers == NULL){
        removeSem(semid);
        ////free(args);
        pthread_attr_destroy(&detachedThread);
        pthread_exit(0);
    }
    for (int i = 0; i < C; i++)
    {
        idOfConsumers[i] = i+1;
    }
    if(terminate){//perror("AA1");
        removeSem(semid);
        pthread_attr_destroy(&detachedThread);
        free(idOfConsumers);
        pthread_exit(0);
    }
    pthread_t* threads = createThreads(C,idOfConsumers);
    if(threads == NULL){
        perror("threads could not be created");
        removeSem(semid);
        ////free(args);
        free(idOfConsumers);
        pthread_attr_destroy(&detachedThread);
        pthread_exit(0);
    }
    
    for (int i = 0; i < C; i++)
    {
        if(pthread_join(threads[i],NULL) != 0){
            perror("pthread_join");
            // do not end the program
        }

    }
    
    removeSem(semid);
    free(idOfConsumers);
    free(threads);
    pthread_attr_destroy(&detachedThread);
    pthread_exit(0);

}
