#include "workerY.h"
void printTime(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
void writeToStdout(char* str){
    struct flock lockToWrite;
    /* lock file to write */
    memset(&lockToWrite,0,sizeof(lockToWrite));
    lockToWrite.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
    printTime();
    printf("%s",str);
    lockToWrite.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite); 
}
void freeMatrix(int ** matrix,int size){
    if(matrix == NULL){
        return;
    }
    for (int i = 0; i < size; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
    
}
sig_atomic_t terminate = 0;
void signalHandler(int sig){
    terminate = 1;
}
int main(int argc, char const *argv[])
{
    if(argc != 4){
        perror("Invalid arguments");
        return -1;
        
    }
    struct sigaction busy;
    memset(&busy,0,sizeof(busy));
    busy.sa_handler = &signalHandler;
    if ((sigemptyset(&busy.sa_mask) == -1) ||  (sigaction(SIGINT,&busy, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    //printf("AAAAAAAAAAAAAAAAAA\n");
    //printf("HEPSI: %s\n",argv[1]);
    int fdRead = atoi(argv[1]);
    int sleepDuration = atoi(argv[2]);
    int poolSize = atoi(argv[3]);
    //printf("argv[1] : %s %d\n",argv[1],fdRead);
    //fflush(stdout);
    //printf("ID: %d sleep: %d\n",getpid(),sleepDuration);
    /*
        If O_CREAT is specified, and a  semaphore  with
       the given name already exists, then mode and value are ignored.

       Semaphore is already created in parent process. Mode and value is ignored.
       semaphore is needed because of that, when the id of client is read, until the other client ID's, all information should be passed to same worker Y process
    */
    //struct flock lock;
    /* Initialize the flock structure */
    //memset(&lock,0,sizeof(lock));
    
    /*Place a read lock on the file. */

    

    
    sem_t * sp = sem_open(SEMAPHORENAMEFORCHILDREN,O_CREAT,S_IRWXU,1);
    for(;;){
        int size = -1,bytesRead=0;
        // mutex down
        sem_wait(sp);
        int idOfClient;
        if(terminate){
            sem_close(sp);
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            return -1;
        }
        while((read(fdRead, &idOfClient, sizeof(int) ) == -1) && errno == EINTR);
        char template[100] = "Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy %d/%d\n";
        if(terminate){
            sem_close(sp);
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            return -1;
        }
        /*if ((r = read(fdRead, &idOfClient, sizeof(int))) != sizeof(int)) { // one of them will read, others will be blocked
                fprintf(stderr, "%d Error reading request; discarding\n",getpid());
                //continue;                   // Either partial read or error 
            }*/
        while((read(fdRead, &size, sizeof(int) ) == -1) && errno == EINTR);

        char str[100];
        memset(str,'\0',100);
        int numberOfbusyWorkers = -1;
        snprintf(str,99,template,getpid(),idOfClient,size,size,numberOfbusyWorkers,poolSize);
        if(terminate){
            sem_close(sp);
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            return -1;
        }
        
        struct flock lockToWrite;
        /* lock file to write */
        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printTime();
        printf("%s",str);
        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite); 

        /*int fd;
        if((fd = open(argv[4],O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR) )== -1 && errno == EINTR ){
            perror("Failed to open log file");
        return -1;
        }
        lock.l_type = F_WRLCK;

        int byteswritten;
        fcntl(fd,F_SETLKW,&lock);
        while((byteswritten = write(fd,str,strlen(str)) == -1) && (errno == EINTR)){
            perror("Could not be written to the log");
        }
        // release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd,F_SETLKW,&lock);*/
        /*if(close(fd) == -1){
            perror("close fd");
            return 1;
        }*/
        /*if ((r = read(fdRead, &size, sizeof(int))) != sizeof(int)) { // one of them will read, others will be blocked
                fprintf(stderr, "%d Error reading request; discarding\n",getpid());
                //continue;                   // Either partial read or error 
            }*/
        
        int k=0;
        int ** matrix = (int**) malloc(sizeof(int*) * size);
        if(matrix == NULL){
            perror("Malloc Error workerY");
            return -1;
        }
        // create space for matrix
        for (int i = 0; i < size; i++)
        {
            matrix[i] = (int*) malloc (sizeof(int) * size);
            if(matrix[i] == NULL){
                perror("Malloc error");
                for (int j = 0; j < i; j++)
                    free(matrix[j]);
                
                return -1;
            }
        }
        
        while(k < size){
            if(terminate){
                sem_close(sp);
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                freeMatrix(matrix,size);
                return -1;
            }
            while (((bytesRead = read(fdRead, matrix[k], sizeof(int) * size )) == -1) && errno == EINTR);
            k++;
            //printf("QQQQQQQQQQQQQ\n");
        }
        // mutex up
        sem_post(sp);
        
        /*while(1){
            while((r = read(fdRead, &val, sizeof(int))) == -1 && errno == EINTR);
            printf("%d ",val);
            if(r == 0){
                break;
            }
        }*/
        //printf("\nProcess %d -> %d %d\n",getpid(),val,r);
        /*for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                printf("p: %d ",matrix[i][j]);
            }
            printf("\n");

        }*/
        //kill(getppid(),SIGUSR1);
        snprintf(clientFifoResult, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_RESULT_TEMPLATE,(long) idOfClient);

        int clientResultFd;
        if(terminate){
                sem_close(sp);
                sem_unlink(SEMAPHORENAMEFORCHILDREN);
                freeMatrix(matrix,size);
                return -1;
            }
        while((clientResultFd = open(clientFifoResult, O_WRONLY)) == -1 && errno == ENOENT );
        if (clientResultFd == -1)
            errExit("open workerY%s", clientFifoResult);
        //char  str[] = "selam";
        
        //Worker PID#778 responding to client PID#669: the matrix IS NOT invertible.
        int result = isInvertible(matrix,size);
        if(result){
            kill(getppid(),SIGUSR1); // increments number of invertible matrixes
        }
        sleep(sleepDuration);
        if(terminate){
            sem_close(sp);
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            freeMatrix(matrix,size);
            return -1;
        }
        // send process id to server to be able to create special fifo to send matrix
        //  sleep(sleepDuration);
        //struct flock lockToWrite;
        /* lock file to write */
        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printTime();
        printf("Worker PID#%d responding to client PID#%d: the matrix %s invertible.\n",getpid(),idOfClient,(result) ? "is" : "IS NOT" );
        fflush(stdout);
        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite); 
        if(write(clientResultFd, &result, sizeof(int)) != sizeof(int))
            fatal("Can't write to server");
        
        /*printf("CHILD Y SLEEPS FOR %d sc\n",sleepDuration);
        
        printf("\nProcess %d Son %d %d\n",getpid(),val,r);*/
        
        kill(getppid(),SIGUSR2); // operation is done
        for(int i=0;i<size;i++){
            free(matrix[i]);
        }
        free(matrix);
        if(terminate){
            sem_close(sp);
            sem_unlink(SEMAPHORENAMEFORCHILDREN);
            return -1;
        }
    }

    //fflush(stdout);
    /*umask(0);                           
    const char * fifoParent = argv[1];

    int serverFd = open(fifoParent, O_RDONLY); // waits here for client
    if (serverFd == -1)
        errExit("open %s", fifoParent);
    int val;
    for(;;){
        if (read(serverFd, &val, sizeof(int))!= sizeof(int)) {
            fprintf(stderr, "Error reading request; discarding\n");
            continue;                   // Either partial read or error 
        }
        printf("%d val: %d",getpid(),val);
        fflush(stdout);
    }*/

    return 0;
}

