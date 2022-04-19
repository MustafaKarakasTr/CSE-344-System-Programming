#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     /* Prototypes for many system calls */
#include <string.h> 
#include <stdlib.h>     /* Prototypes of commonly used library functions,
                           plus EXIT_SUCCESS and EXIT_FAILURE constants */
#include<signal.h>
#include<semaphore.h>
#include "error_functions.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdlib.h> 
#include <time.h> 
#include <sys/time.h>
#include <sys/mman.h>

#define BUFSIZE 10
#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
#define CLIENT_FIFO_RESULT_TEMPLATE "/tmp/result_cl.%ld"
#define CHILD_FIFO_TEMPLATE "/tmp/childP_cl.%ld"
                                /* Template for building client FIFO name */
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
#define BD_NO_CHDIR  01
#define BD_NO_CLOSE_FILES  02
#define BD_NO_REOPEN_STD_FDS  04
#define BD_NO_UMASK0 010
#define BD_MAX_CLOSE 8192

#define MAGICNUMBER -2435701
static char clientFifo[CLIENT_FIFO_NAME_LEN];
sig_atomic_t numberOfBusyChildProcesses=0;
sig_atomic_t terminate = 0;
sem_t * globalSp =NULL;
pid_t serverZId=0;
char SERVER_FIFO[] = "/tmp/seqnum_sv";
static char sharedMemory[] = "/doubleInstantiationChecker8";
sig_atomic_t numberOfInvertibleMatrixes = 0;
int numberOfMatrixes = 0;
int numberOfforwarded = 0;
int becomeDaemon(int flags);

void waitForChildren(int numberOfChildren){

    for(int i=0;i<numberOfChildren;i++){
        wait(NULL);
    }
    if(serverZId != 0){
        kill(serverZId,SIGINT);
    }
    //while (wait(NULL) > 0);
}
static void             /* Invoked on exit to delete client FIFO */
removeSemaphore(void)
{
    
    if(globalSp != NULL){
        sem_close(globalSp);
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
    }
    unlink(SERVER_FIFO);
}
void printTime(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
void signalHandler(int signalNumber){
    if(signalNumber == SIGINT){
        terminate = 1;
        struct flock lockToWrite;

        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printTime();
        //printf("%s",str);
        printf("SIGINT received, terminating Z and exiting server Y. Total requests handled: %d, %d invertible, %d not. %d requests were forwarded.\n",numberOfMatrixes,numberOfInvertibleMatrixes,numberOfMatrixes-numberOfInvertibleMatrixes,numberOfforwarded );
        fflush(stdout);
        
        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        
    } else if(signalNumber == SIGUSR1 ){
        numberOfInvertibleMatrixes++;
    } else if(signalNumber == SIGUSR2 ){
        numberOfBusyChildProcesses--;

    }
}

int main2(){
    unlink(SERVER_FIFO);
    return 0;
}

//------------------

int createSharedSpace(){
    //char sharedMemory[] = "sharedMemory";
    int fd = shm_open(sharedMemory, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if(fd == -1){
        return -1;
    }
    return fd;
}
void* expandSharedSpace(int fd, int oldSize,int addedSize){
    int totalSize = oldSize+addedSize;
    if (ftruncate(fd, totalSize) == -1){
        perror("ftruncate");
        return NULL; 
    }
    void *addr = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fd, 0);
    if (addr == MAP_FAILED || addr == NULL){
        perror("mmap");
        return NULL;
    }
    return addr;
}
//----------------
int main(int argc, char *argv[])
{   
    if(argc != 11){
        perror("Invalid arguments.\nCorrect the format is : ./serverY -s pathToServerFifo -o pathToLogFile –p poolSize -r poolSize2 -t 2");
        return -1;
    }

    /* Double instantiation check*/
    int sharedMemFd = createSharedSpace();
    if(sharedMemFd == -1){
        perror("Double instantiation is not allowed.");
        return -1;
    }
    
    //becomeDaemon(BD_NO_CHDIR | BD_NO_UMASK0 | BD_NO_CLOSE_FILES); // makes program stuck
    int toFd;
    unlink(argv[4]);
    if((toFd = open(argv[4],O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR) )== -1 && errno == EINTR ){
        perror("Failed to open log file");
        shm_unlink(sharedMemory);
        return -1;
    }
    char template[100] = "Server Y (%s, p=%s, t=%s) started\n";
    char str[100];
    memset(str,'\0',100);
    snprintf(str,99,template,argv[4],argv[6],argv[10]);
    /*int logFd = open(argv[4],O_CREAT | O_WRONLY,S_IRWXU);
    if(logFd == -1){
        perror("log file could not be opened\n");
        return -1;
    }
    if(close(logFd) == -1){
        perror("close");
        return -1;
    }*/
    int byteswritten = 0;
    //char str[] = "Selamun aleykum";
    while((byteswritten = write(toFd,str,strlen(str)) == -1) && (errno == EINTR));
    printTime();
    printf("%s",str);
    fflush(stdout);

    //close(toFd);
    
    struct sigaction busy;
    memset(&busy,0,sizeof(busy));
    busy.sa_handler = &signalHandler;
    if ((sigemptyset(&busy.sa_mask) == -1) || (sigaction(SIGUSR1,&busy, NULL) == -1)||  (sigaction(SIGUSR2,&busy,NULL)==-1) ||(sigaction(SIGINT,&busy, NULL) == -1)  )
    {    
        perror("Failed to install SIGINT signal handler");
        shm_unlink(sharedMemory);
        return -1;
    }
    /*sigaction(SIGUSR1,&busy,NULL);
    sigaction(SIGUSR2,&busy,NULL);
    sigaction(SIGINT,&busy, NULL);
    */
    sem_unlink(SEMAPHORENAMEFORCHILDREN);
    sem_t * sp = sem_open(SEMAPHORENAMEFORCHILDREN,O_CREAT,S_IRWXU,1);
    globalSp = sp;
    if (atexit(removeSemaphore) != 0)
    {
        perror("atexit");
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
        shm_unlink(sharedMemory);
        return -1;
    }

    // when SIGINT comes, sp must be closed
    /*struct sigaction notBusy;
    memset(&notBusy,0,sizeof(notBusy));
    notBusy.sa_handler = &childProcessIsNotBusy;
    sigaction(SIGUSR2,&notBusy,NULL);*/


    int poolSize1 = atoi(argv[6]); // poolsize for Y
    if(poolSize1 < 2){
        perror("Pool size of serverY is not enough. Please give a number greater than or equal to 2\n");
        shm_unlink(sharedMemory);
        return -1;
    }
    int poolSize2 = atoi(argv[8]); // poolsize for Z
    if(poolSize2 < 2){
        perror("Pool size of serverZ is not enough. Please give a number greater than or equal to 2\n");
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
        shm_unlink(sharedMemory);
        return -1;
    }
    int sleepDuration = atoi(argv[10]);
    int filedes[2];
    if(pipe(filedes) == -1)
    {
        perror("pipe");
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
        shm_unlink(sharedMemory);
        return -1;
    }


    /*if(close(filedes[0])) // child will read
                errExit("close");*/
    for(int i=0;i<poolSize1;i++){ 
        // fifo is being created to communicate with child process 
        /*char fifoName[CLIENT_FIFO_NAME_LEN];
        snprintf(fifoName, CLIENT_FIFO_NAME_LEN, CHILD_FIFO_TEMPLATE,(long) i); // each fifo name holds its index to becoma distinct
        umask(0); 
        if (mkfifo(fifoName, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
            errExit("mkfifo %s", fifoName);*/
        
        
        /*if(write(filedes[1], &id, sizeof(int)) != sizeof(int))
            fatal("Can't write to server");*/
        char filedesWriteStr[20];
        char sleepDurationStr[20];
        char poolSizeStr[20];

        char filedesWriteStrTemplate[]="%ld";

        snprintf(filedesWriteStr, 20, filedesWriteStrTemplate,(long) filedes[0]);
        snprintf(sleepDurationStr, 20, filedesWriteStrTemplate,(long) sleepDuration);
        snprintf(poolSizeStr, 20, filedesWriteStrTemplate,(long) poolSize1);

        pid_t forkId = fork();
        if(forkId == -1)
        {
            perror("Fork");
            close(filedes[0]);
            close(filedes[1]);
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            shm_unlink(sharedMemory);
            return -1;
        }
        else if(forkId == 0){
            
            // fifoName will be used for communicating with child process
            
            if(close(filedes[1]) == -1) // child will read
            {
                perror("close");
                close(filedes[0]);
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                return -1;
            }
            //perror("AAAAAAAAAAAAAAAAAAABBBBB\n");
             // each fifo name holds its index to becoma distinct
            // //printf("CHILD ICI READDEN ONCE\n"); 
            /*if (r = read(filedes[0], &val, sizeof(int)) != sizeof(int)) { // one of them will read, others will be blocked
                fprintf(stderr, "Error reading request; discarding\n");
            //continue;                   // Either partial read or error 
            }*/
           //printf("ID: %d CHILDa: %d\n",getpid(),val);
            char * arr[] = {"workerY",filedesWriteStr,sleepDurationStr,poolSizeStr,NULL};
            execv(arr[0],arr);
        } 
        
    }
    // create Z child
    int filedesForZ[2];
    if(pipe(filedesForZ) == -1){
        perror("pipe");
        close(filedes[0]);
        close(filedes[1]);
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
        shm_unlink(sharedMemory);
        return -1;
    }
    int zId = fork();
    if(zId == -1){
        perror("fork");
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
        shm_unlink(sharedMemory);
        close(filedes[0]);
        close(filedes[1]);
        close(filedesForZ[0]);
        close(filedesForZ[1]);
        
        return -1;
    }
    if(zId == 0){
        // execv Z program
        if(close(filedesForZ[1]) == -1) // Z will read
        {
            perror("close");
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            close(filedes[0]);
            close(filedes[1]);
            close(filedesForZ[0]);
            
            return -1;
        }
        

             // each fifo name holds its index to becoma distinct
        char filedesWriteStr[20];
        char poolSizeForZStr[20];
        char sleepDurationStr[20];
        char filedesWriteStrTemplate[]="%ld";

        snprintf(filedesWriteStr, 20, filedesWriteStrTemplate,(long) filedesForZ[0]);
        snprintf(poolSizeForZStr, 20, filedesWriteStrTemplate,(long) poolSize2);
        snprintf(sleepDurationStr, 20, filedesWriteStrTemplate,(long) sleepDuration);
        //printf("ID: %d Y ICINDE Z NIN FD: %d\n",getpid(),filedesForZ[0]);
        strcpy(str,"Instantiated server Z\n");
        str[strlen("Instantiated server Z\n")] = '\0';
        while((byteswritten = write(toFd,str,strlen(str)) == -1) && (errno == EINTR));
        printTime();
        printf("%s",str);
        fflush(stdout);
        close(toFd);
        char * arr[] = {"serverZ",filedesWriteStr,poolSizeForZStr,sleepDurationStr,argv[4],NULL};
        execv(arr[0],arr);
    }
    serverZId = zId;
    //sleep(10);
    //char buf[BUFSIZE]; 
    int serverFd, dummyFd, clientFd;
    //char clientFifo[CLIENT_FIFO_NAME_LEN];

    /* Create well-known FIFO, and open it for reading */

    umask(0);   
                            /* So we get the permissions we want */
    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1
            && errno != EEXIST){
                perror("mkfifo");
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                close(filedes[0]);
                close(filedes[1]);
                close(filedesForZ[0]);
                close(filedesForZ[1]);
                waitForChildren(poolSize1);
                shm_unlink(sharedMemory);
                return -1;
            }
        
    while((serverFd = open(SERVER_FIFO, O_RDONLY)) == -1 && errno == EINTR){
        if(terminate){
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            close(filedes[0]);
            close(filedes[1]);
            close(filedesForZ[0]);
            close(filedesForZ[1]);
            waitForChildren(poolSize1);
            shm_unlink(sharedMemory);
            return -1;
        }
        continue;
    } // waits here for client

    if (serverFd == -1){
        perror("open");
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
        close(filedes[0]);
        close(filedes[1]);
        close(filedesForZ[0]);
        close(filedesForZ[1]);
        waitForChildren(poolSize1);
        shm_unlink(sharedMemory);
        return -1;
    }

    /* Open an extra write descriptor, so that we never see EOF */

    dummyFd = open(SERVER_FIFO, O_WRONLY); // even if the client closes the write end, server will not take SIGPIPE
    if (dummyFd == -1)
    {
        perror("open");
        sem_unlink(SEMAPHORENAMEFORCHILDREN);
        close(filedes[0]);
        close(filedes[1]);
        close(filedesForZ[0]);
        close(filedesForZ[1]);
        waitForChildren(poolSize1);
        shm_unlink(sharedMemory);

        return -1;
    }

    // change with sigaction
    //if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    errExit("signal");
    int idOfClient = 0;
    for (;;) {                          /* Read requests and send responses */
        if(terminate){

            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            close(filedes[0]);
            close(filedes[1]);
            close(filedesForZ[0]);
            close(filedesForZ[1]);
            waitForChildren(poolSize1);
            shm_unlink(sharedMemory);

            return -1;
        }
        while ((read(serverFd, &idOfClient, sizeof(int)) == -1) && errno == EINTR ){
            
            //perror("Client Okuma Hata\n");
            if(terminate){

                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                close(filedes[0]);
                close(filedes[1]);
                close(filedesForZ[0]);
                close(filedesForZ[1]);
                waitForChildren(poolSize1);
                shm_unlink(sharedMemory);

                return -1;
            }
        } /*{ // reads ID of client
                
            fprintf(stderr, "Error reading id request; discarding %d\n",);
            continue;                   
        }*/
        if(terminate){
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            close(filedes[0]);
            close(filedes[1]);
            close(filedesForZ[0]);
            close(filedesForZ[1]);
            waitForChildren(poolSize1);
            shm_unlink(sharedMemory);

            return -1;
        }
        numberOfBusyChildProcesses++; // new client is connected
       //printf("%d\n",idOfClient);
        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,
            (long) idOfClient);
        clientFd = open(clientFifo, O_RDONLY);
        if (clientFd == -1) {           /* Open failed, give up on client */
            perror("open clientFifo");
            continue;
        }
        if(terminate){
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            close(filedes[0]);
            close(filedes[1]);
            close(filedesForZ[0]);
            close(filedesForZ[1]);
            waitForChildren(poolSize1);
            shm_unlink(sharedMemory);

            return -1;
        }
        int size = -1;
        if (read(clientFd, &size, sizeof(int)) // reads size of client's matrix
                != sizeof(int)) {
            perror("Error reading size request; discarding\n");
            continue;                   /* Either partial read or error */
        }
        if(terminate){
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            close(filedes[0]);
            close(filedes[1]);
            close(filedesForZ[0]);
            close(filedesForZ[1]);
            waitForChildren(poolSize1);
            shm_unlink(sharedMemory);

            return -1;
        }
       //printf("Size: %d\n",size);
        int ** matrix = (int**) malloc(sizeof(int*) * size);
        if(matrix == NULL){
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            close(filedes[0]);
            close(filedes[1]);
            close(filedesForZ[0]);
            close(filedesForZ[1]);
            waitForChildren(poolSize1);
            perror("Malloc Error");
            shm_unlink(sharedMemory);

            return -1;
        }
        // create space for matrix
        for (int i = 0; i < size; i++)
        {
            matrix[i] = (int*) malloc (sizeof(int) * size);
            if(matrix[i] == NULL){
                perror("Malloc error");
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                close(filedes[0]);
                close(filedes[1]);
                close(filedesForZ[0]);
                close(filedesForZ[1]);
                waitForChildren(poolSize1);
                for (int j = 0; j < i; j++)
                    free(matrix[j]);
                shm_unlink(sharedMemory);
                
                return -1;
            }
        }
        
       //printf("READ'Den once\n");
        // if there is nothing to read, it will be blocked
        int bytesRead;
        // read returns number of bytes read. if it is 0, then EOF is reached.
        
        /*
         while(((bytesread = read(fromfd, buf,BLKSIZE-1)) == -1)&&
            (errno == EINTR));
        */
        // read matrix from client fifo
        int k=0;

        while(k < size){
            if(terminate){
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                close(filedes[0]);
                close(filedes[1]);
                close(filedesForZ[0]);
                close(filedesForZ[1]);
                waitForChildren(poolSize1);
                shm_unlink(sharedMemory);

                for(int i=0;i<size;i++){
                    free(matrix[i]);
                }
                free(matrix);
                return -1;
            }
            while (((bytesRead = read(clientFd, matrix[k], sizeof(int) * size )) == -1) && errno == EINTR){
                perror("matrix read error\n");
                if(terminate){
                    sem_unlink(SEMAPHORENAMEFORCHILDREN);
                    close(filedes[0]);
                    close(filedes[1]);
                    close(filedesForZ[0]);
                    close(filedesForZ[1]);
                    waitForChildren(poolSize1);
                    shm_unlink(sharedMemory);

                    for(int i=0;i<size;i++){
                        free(matrix[i]);
                    }
                    free(matrix);
                    return -1;
                }
            }
            k++;
           //printf("QQQQQQQQQQQQQ\n");
            if(terminate){
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                close(filedes[0]);
                close(filedes[1]);
                close(filedesForZ[0]);
                close(filedesForZ[1]);
                waitForChildren(poolSize1);
                shm_unlink(sharedMemory);

                for(int i=0;i<size;i++){
                    free(matrix[i]);
                }
                free(matrix);

                return -1;
            }
            
        }
        numberOfMatrixes++;
        // write matrix to pipe if there is available children
        if(numberOfBusyChildProcesses-1 != poolSize1){
            k=0;
            if(terminate){
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                close(filedes[0]);
                close(filedes[1]);
                close(filedesForZ[0]);
                close(filedesForZ[1]);
                waitForChildren(poolSize1);
                shm_unlink(sharedMemory);
    
                for(int i=0;i<size;i++){
                    free(matrix[i]);
                }
                free(matrix);
                return -1;
            }
            while((write(filedes[1], &idOfClient, sizeof(int) ) == -1) && errno == EINTR);
            while((write(filedes[1], &size, sizeof(int) ) == -1) && errno == EINTR);
            while(k < size){
                if(terminate){
                    sem_unlink(SEMAPHORENAMEFORCHILDREN);
                    close(filedes[0]);
                    close(filedes[1]);
                    close(filedesForZ[0]);
                    close(filedesForZ[1]);
                    waitForChildren(poolSize1);
                    shm_unlink(sharedMemory);
        
                    for(int i=0;i<size;i++){
                        free(matrix[i]);
                    }
                    free(matrix);
                    return -1;
                }
                //while (((bytesRead = read(clientFd, matrix[k], sizeof(int) * size )) == -1) && errno == EINTR);
                while((write(filedes[1], matrix[k], sizeof(int) * size) == -1) && errno == EINTR);
                    //fatal("Can't write to worker Y");
                if(terminate){
                    sem_unlink(SEMAPHORENAMEFORCHILDREN);
                    close(filedes[0]);
                    close(filedes[1]);
                    close(filedesForZ[0]);
                    close(filedesForZ[1]);
                    waitForChildren(poolSize1);
                    shm_unlink(sharedMemory);
                
                    for(int i=0;i<size;i++){
                        free(matrix[i]);
                    }
                    free(matrix);
                    return -1;
                }
                k++;
               //printf("QQQQQQQQQQQQQ\n");
            }
        } else{ // send it to Z
            numberOfforwarded++;
            //char str[100];
            numberOfBusyChildProcesses--;
            struct flock lockToWrite;
            /* lock file to write */
            memset(&lockToWrite,0,sizeof(lockToWrite));
            lockToWrite.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
            printTime();
            //printf("%s",str);
            printf("Forwarding request of client PID#%d to serverZ, matrix size %dx%d, pool busy %d/%d\n",idOfClient,size,size,numberOfBusyChildProcesses,numberOfBusyChildProcesses);
            fflush(stdout);
            
            lockToWrite.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite); 
            //Forwarding request of client PID#700 to serverZ, matrix size 3x3, pool busy 4/4
           //printf("HER COCUK DOLU Z YE GONDERILMELI\n");
            /*
            // Z sanki bu cevabı verıyormus gibi:
            snprintf(clientFifoResult, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_RESULT_TEMPLATE,(long) idOfClient);
            while((clientResultFd = open(clientFifoResult, O_WRONLY)) == -1 && errno == ENOENT );
            //clientResultFd = open(clientFifoResult, O_WRONLY);
            if (clientResultFd == -1)
                errExit("open in Z %s", clientFifoResult);
            //char  str[] = "selam";
            
            int id = 1453;
            // send process id to server to be able to create special fifo to send matrix
            if(write(clientResultFd, &id, sizeof(int)) != sizeof(int))
                fatal("Can't write to server");
            */
            k=0;
            if(terminate){
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                close(filedes[0]);
                close(filedes[1]);
                close(filedesForZ[0]);
                close(filedesForZ[1]);
                waitForChildren(poolSize1);
                shm_unlink(sharedMemory);
                
                return -1;
            }
            while((write(filedesForZ[1], &idOfClient, sizeof(int) ) == -1) && errno == EINTR);
            while((write(filedesForZ[1], &size, sizeof(int) ) == -1) && errno == EINTR);
            while(k < size){
                if(terminate){
                    sem_unlink(SEMAPHORENAMEFORCHILDREN);
                    close(filedes[0]);
                    close(filedes[1]);
                    close(filedesForZ[0]);
                    close(filedesForZ[1]);
                    waitForChildren(poolSize1);
                    shm_unlink(sharedMemory);
                
                    return -1;
                
                }
                //while (((bytesRead = read(clientFd, matrix[k], sizeof(int) * size )) == -1) && errno == EINTR);
                while((write(filedesForZ[1], matrix[k], sizeof(int) * size) == -1) && errno == EINTR);
                    //fatal("Can't write to worker Y");
                if(terminate){
                    sem_unlink(SEMAPHORENAMEFORCHILDREN);
                    close(filedes[0]);
                    close(filedes[1]);
                    close(filedesForZ[0]);
                    close(filedesForZ[1]);
                    waitForChildren(poolSize1);
                    shm_unlink(sharedMemory);
                    
                    return -1;
                }
                k++;
            }
        }
        for(int i=0;i<size;i++){
            free(matrix[i]);
        }
        free(matrix);

        }
}
int becomeDaemon(int flags){
    
    int maxfd, fd;
    switch (fork()) {
        case -1: return -1;
        case  0: break;
        default: _exit(EXIT_SUCCESS);
    }

    if (setsid() == -1)
        return -1; 

    switch (fork()) {
        case -1: return -1;
        case  0: break;
        default: _exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))
        umask(0);

    if (!(flags & BD_NO_CHDIR))
        chdir("/");    

    if (!(flags & BD_NO_CLOSE_FILES)) {
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1)
            maxfd = BD_MAX_CLOSE;
    }
    for (fd = 0; fd < maxfd; fd++)
        close(fd);
    
    if (!(flags & BD_NO_REOPEN_STD_FDS)) {
        close(STDIN_FILENO);

        fd = open("/dev/null", O_RDWR);

        if (fd != STDIN_FILENO)
            return -1;

        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -1;
    }
    return 0;
}