#include "serverZ.h"
//void *sharedMemoryAddr = NULL;
sig_atomic_t terminate = 0;
sem_t * mutex =NULL;
sem_t * full =NULL;
sem_t * empty=NULL;
int numberOfForwarded = 0;
int numberOfInvertible = 0;

void printTime(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}


void signalHandler(int sigNum){
    if(sigNum == SIGINT){
        /*struct flock lockToWrite;
        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printTime();
        //printf("%s",str);
        printf("Z:SIGINT received, exiting server Z. Total requests handled %d, %d invertible, %d not.\n",numberOfForwarded,numberOfInvertible,numberOfForwarded-numberOfInvertible);
        fflush(stdout);
        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);*/
        terminate = 1;
    }
    else if(sigNum == SIGUSR1){
        kill(getppid(),sigNum); // workerZ sends signal to its parent serverZ, serverZ passes it to its parent serverY 
    }
    
}
void waitForChildren(int numberOfChildren){
   for(int i=0;i<numberOfChildren;i++){
        wait(NULL);
    }
}

int createSharedSpace(){
    //char sharedMemory[] = "sharedMemory";
    int fd = shm_open(sharedMemory, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(fd == -1){
        perror("shm_open");
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
static void removeSemaphore(void){
    sem_close(mutex);
    unlink(MUTEX);

    sem_close(full);
    unlink(FULL);

    sem_close(empty);
    unlink(EMPTY);
}
int main(int argc, char const *argv[])
{
    struct sigaction sigAction;
    memset(&sigAction,0,sizeof(sigAction));
    sigAction.sa_handler = &signalHandler;
    if ((sigemptyset(&sigAction.sa_mask) == -1)  
        ||(sigaction(SIGINT,&sigAction, NULL) == -1) 
        ||(sigaction(SIGUSR1,&sigAction, NULL) == -1)){
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    //Z:Server Z (../../demo/21/exam/y.log, t=2, r=4) started

    //printf("Z ICINDE FD: %s\n",argv[1]);
    int fdRead = atoi(argv[1]);
    int poolsize = atoi(argv[2]);
    //poolSizeGlobal = poolsize;
    int sleepDuration = atoi(argv[3]);

    struct flock lockToWrite;
    /* lock file to write */
    memset(&lockToWrite,0,sizeof(lockToWrite));
    lockToWrite.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
    printTime();
    printf("Z:Server Z (%s, t=%d, r=%d) started\n",argv[4],sleepDuration,poolsize);
    fflush(stdout);

    lockToWrite.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite); 

    //printf("Z : ID: %d sleep: %d\n",getpid(),sleepDuration);

    sem_unlink(MUTEX);
    sem_unlink(FULL);
    sem_unlink(EMPTY);


    mutex = sem_open(MUTEX,O_CREAT | O_EXCL,S_IRWXU,1);
    //printf("MUTEX::: SERVER: %p\n",mutex);
    // starts from 0 because initially shared memory is empty
    full = sem_open(FULL,O_CREAT,S_IRWXU,0);
    empty = sem_open(EMPTY,O_CREAT,S_IRWXU,N);
    /*int m,e;
    sem_getvalue(mutex,&m);
    sem_getvalue(empty,&e);
    */
    //printf("\nmutex ZNIN BASI: %d\n",m);
    //printf("empty ZNIN BASI: %d\n",e);


    /*int id = fork();
    if(id == 0){
        char clientId[20];
        char sharedMemoryName[20];
        char * argvExec[] = {"workerZ",sharedMemoryName,clientId,NULL};
        printf("AAAAAAAAAAa");
        fflush(stdout);
        execv(argvExec[0],argvExec);
    }
    return 0;*/
    // create shared space
    int sharedMemFd = createSharedSpace();
    if(sharedMemFd == -1){
        perror("Shared memory could not be opened");
        sem_unlink(MUTEX);
        sem_unlink(FULL);
        sem_unlink(EMPTY);
        return -1;
    }
    //indexInSharedMemory = 0;
    //printf("sharedMemFd: %d, sizeOfSharedMemory: %d\n",sharedMemFd,sizeOfSharedMemory);
    void *sharedMemoryAddr = expandSharedSpace(sharedMemFd,0,sizeOfSharedMemory); 
    if(sharedMemoryAddr == NULL){
        shm_unlink(sharedMemory);
        perror("Share space could not be expanded");
        return -1;
    }
    int endOfSharedMem = -1;
    //printf("MAIN: sharedMemoryAddr: %p\n",sharedMemoryAddr);
    // put -1 to mention that shared space is empty
    memcpy(sharedMemoryAddr, &endOfSharedMem, sizeof(int));
    //printf("MAIN: sharedMemoryAddr: %p\n",sharedMemoryAddr);
    
    
    // save shared memory's address to shared memory
    // to be able to read shared space sync. we need index in it, seen from all children processes and parent
    //memcpy(sharedMemoryAddr, &addr, sizeof(void*));
    //int magicNum = 9421;
    
    /*for(int i=0;i<10;i++){
        memcpy(sharedMemoryAddr+i*sizeof(int), &magicNum, sizeof(int));
        magicNum++;
    }*/

    //memcpy(sharedMemoryAddr+sizeof(int), str, strlen(str));
    //printf("SHARED CONTENT: ");
    //write(STDOUT_FILENO, sharedMemoryAddr, strlen(str));
    

    /*printf("\n");
    printf("IN SERVER Z: sharedMemoryAddr: %p",sharedMemoryAddr);   
    fflush(stdout);
    */
    for(int i=0;i<poolsize;i++){
        pid_t id = fork();
        if(id == -1){
            perror("fork error");
            shm_unlink(sharedMemory);
            return -1;
        }
        if(id == 0){ // child
            //char clientId[20];
            //printf("%d PROCEESS CALLED\n",getpid());
           // printf("sharedMemoryAddr: %p\n",sharedMemoryAddr);
            int status = workerZProcess(sharedMemoryAddr,sleepDuration,sharedMemFd,poolsize);
            //kill(getpid(),SIGINT);
            return status;
            /*char *sharedMemoryName = sharedMemory;
            char initialSizeOfSharedMemoryStr[10];
            snprintf(initialSizeOfSharedMemoryStr,10,"%d",sizeOfSharedMemory);
            char * argvExec[] = {"workerZ",sharedMemoryName,initialSizeOfSharedMemoryStr,NULL};
            printf(" Z CHILD: %p",sharedMemoryAddr);
            char buf[10];
            memset(buf,'\0',10);*/
            /*for (int i = 0; i < 10; i++)
            {
            int mag;
                memcpy(&mag, sharedMemoryAddr+i*sizeof(int), sizeof(int));
                //int mag = atoi(buf);
                printf("mag-> %d\n",mag);
            }
            */

            /*for(int i=0;i<4;i++)
                write(STDOUT_FILENO, sharedMemoryAddr+sizeof(int), strlen(str));
            fflush(stdout);*/
            
            //execv(argvExec[0],argvExec);
        } else{ // parent

        }
    }
    if(terminate){
        shm_unlink(sharedMemory);
        waitForChildren(poolsize);
        return -1;
    }
    if (atexit(removeSemaphore) != 0){
        shm_unlink(sharedMemory);
        perror("atexit");
        return -1;
    }
    for(;;){
        int size = -1,bytesRead=0;
        // mutex down
        
        if(terminate){
            shm_unlink(sharedMemory);
            waitForChildren(poolsize);
            return -1;
        }
        int idOfClient;
        
        while((read(fdRead, &idOfClient, sizeof(int) ) == -1) && errno == EINTR);

        /*if ((r = read(fdRead, &idOfClient, sizeof(int))) != sizeof(int)) { // one of them will read, others will be blocked
                fprintf(stderr, "%d Error reading request; discarding\n",getpid());
                //continue;                   // Either partial read or error 
            }*/
        while((read(fdRead, &size, sizeof(int) ) == -1) && errno == EINTR);
        if(terminate){
            waitForChildren(poolsize);
            shm_unlink(sharedMemory);
            return -1;
        }
        int requiredSize = (size*size+3)*sizeof(int);
        if(sizeOfSharedMemory < requiredSize){ //shared memory holds size^2 + 3 number
            //sizeOfSharedMemory
            //sharedMemoryAddr = expandSharedSpace(sharedMemFd,0,sizeOfSharedMemory); 
        }
        //Z:Worker PID#800 is handling client PID#700, matrix size 5x5, pool busy 1/4
        /*struct flock lockToWrite;
        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printTime();
        printf("Z:Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy -1/%d\n",getpid(),idOfClient,size,size,poolsize);
        fflush(stdout);
        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite); */
        int k=0;
        int ** matrix = (int**) malloc(sizeof(int*) * size);
        if(matrix == NULL){
            if(!terminate)
                perror("Malloc Error");
            waitForChildren(poolsize);
            shm_unlink(sharedMemory);
            return -1;
        }
        // create space for matrix
        for (int i = 0; i < size; i++)
        {
            matrix[i] = (int*) malloc (sizeof(int) * size);
            if(matrix[i] == NULL){
                shm_unlink(sharedMemory);
                perror("Malloc error");
                waitForChildren(poolsize);
                for (int j = 0; j < i; j++)
                    free(matrix[j]);
                free(matrix);
                return -1;
            }
        }
        // matrix is taken from pipe which is connected to serverY
        while(k < size){
            while (((bytesRead = read(fdRead, matrix[k], sizeof(int) * size )) == -1) && errno == EINTR);
            k++;
        }
        if(terminate){
            shm_unlink(sharedMemory);
            for(int i=0;i<size;i++){
                free(matrix[i]);
            }
            free(matrix);
            waitForChildren(poolsize);
            return -1;
        }
        

        //int val = 999;

        // Item is produced.
        //perror("BLOCKLANDI MI YOKSA"); // cannot pass from here
        //int mutex_val = -45,emprtt=12;
        if(terminate){
            shm_unlink(sharedMemory);
            for(int i=0;i<size;i++){
                free(matrix[i]);
            }
            free(matrix);
            waitForChildren(poolsize);
            return -1;
        }
        //sem_getvalue(mutex,&mutex_val);
        //sem_getvalue(empty,&emprtt);
        /*printf("mutex: %d",mutex_val);
        printf("empty: %d",emprtt);*/
        /*int* temp2 =(int*) sharedMemoryAddr;
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                printf("q: %d ",temp2[i*size+j]);
            }
            printf("\n");

        }
        fflush(stdout);*/
        if(terminate){
            shm_unlink(sharedMemory);
            waitForChildren(poolsize);
            for(int i=0;i<size;i++){
                free(matrix[i]);
            }
            free(matrix);
            return -1;
        }
        // producer starts inserting values to shared memory 
        sem_wait(empty);
        sem_wait(mutex);
        //perror("BLOCKdan sonra");
        if(terminate){
            waitForChildren(poolsize);
            shm_unlink(sharedMemory);
            for(int i=0;i<size;i++){
                free(matrix[i]);
            }
            free(matrix);            
            return -1;
        }
        // save elements to shared memory
        int* tempAddressInShrMem =(int*) sharedMemoryAddr;

        int id_temp = 0;
        int size_temp = 0;
        while(1){
            if(terminate){
                waitForChildren(poolsize);
                for(int i=0;i<size;i++){
                    free(matrix[i]);
                }
                free(matrix);
                shm_unlink(sharedMemory);
                return -1;
            }
            memcpy(&id_temp, tempAddressInShrMem, sizeof(int));
            /*if(id_temp != -1){
                perror("HHHHHHHHHHHHHHHHHHHhh");
                return -1;
            }*/
            
            if(id_temp == -1){
                // read until end of shared memory
                // after this while loop, pointer shows the end of shared memory
                break;
            }
            tempAddressInShrMem++;
            memcpy(&size_temp, tempAddressInShrMem++, sizeof(int));
            int sizeOfMatrix = size_temp*size_temp;
            /*for(int index = 0;index < sizeOfMatrix;index++){
                tempAddressInShrMem++;
            }*/
            tempAddressInShrMem += sizeOfMatrix; // goes end of the 

        }

        // tempAddressInShrMem shows index of -1 which is in the end of shared space
        // now new matrix can be inserted
        
        // save id of client
        memcpy(tempAddressInShrMem++,&idOfClient, sizeof(int));
        // save size of matrix
        memcpy(tempAddressInShrMem++,&size, sizeof(int));
        // save matrix
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                
                memcpy(tempAddressInShrMem++,&matrix[i][j],sizeof(int));
            }
            
        }
        // insert -1 to make clear it is end of shared memory.
        int temp = -1;
        memcpy(tempAddressInShrMem++,&temp, sizeof(int));
        //printf("BUTUN MATRIX:\n");
        //int * p = (int*) sharedMemoryAddr;
        /*for(int i=0;i<160;i++){
            printf("%d ",p[i]);
        }
        
        printf("SERVERZ BASTI\n");*/
        //int * storage =(int*) sharedMemoryAddr;
        /*perror("BUYUTMEDEN ONCE");
        
        for(int i=0;i<sizeOfSharedMemory;i++){
            printf("%d ",((int*)sharedMemoryAddr)[i]);
        }
        printf("\n");*/
        /*
        perror("BUYUTMEDEN SONRA");
        sizeOfSharedMemory *= 2;
        sharedMemoryAddr = expandSharedSpace(sharedMemFd,0,sizeOfSharedMemory); 
        for(int i=0;i<sizeOfSharedMemory;i++){
            printf("%d ",((int*)sharedMemoryAddr)[i]);
        }
        printf("\n");
        if(sharedMemoryAddr == NULL){
            shm_unlink(sharedMemory);
            perror("Share space could not be expanded");
            return -1;
        }*/
        sem_post(mutex);
        sem_post(full);
        
        if(terminate){
            waitForChildren(poolsize);
            shm_unlink(sharedMemory);
            for(int i=0;i<size;i++){
                free(matrix[i]);
            }
            free(matrix);
            return -1;
        }
        /*for(int i=0;i<size*size+3;i++){
            int k = 0;
            memcpy(&k,storage, sizeof(int));
            printf("%d ",k);
            storage++;
        }*/ 
        //kill(getppid(),SIGUSR1);
        /*
        snprintf(clientFifoResult, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_RESULT_TEMPLATE,(long) idOfClient);
        
        
        int clientResultFd;
        while((clientResultFd = open(clientFifoResult, O_WRONLY)) == -1 && errno == ENOENT );
        if (clientResultFd == -1)
            errExit("open workerY%s", clientFifoResult);
        //char  str[] = "selam";
        
        int result = isInvertible(matrix,size);
        result = 4;
        // send process id to server to be able to create special fifo to send matrix
        //sleep(5);
        if(write(clientResultFd, &result, sizeof(int)) != sizeof(int))
            fatal("Can't write to server");
        if(terminate){
            return -1;
        }
        sleep(sleepDuration);
        if(terminate){
            return -1;
        }*/
        //printf("\nProcess IN Z %d Son %d %d\n",getpid(),val,r);
        for(int i=0;i<size;i++){
            free(matrix[i]);
        }
        free(matrix);
        
    }
    return 0;
}

int workerZProcess(void *sharedMemoryAddr,int sleepDuration,int sharedMemFd,int poolsize){

    sem_t *mutexChild = sem_open(MUTEX,O_CREAT,S_IRWXU,1);
    mutex= mutexChild;
    //printf("MUTEX::: WORKER: %p\n",mutexChild);
    sem_t *fullChild = sem_open(FULL,O_CREAT,S_IRWXU,0);
    full =fullChild;
    sem_t * emptyChild = sem_open(EMPTY,O_CREAT,S_IRWXU,N);
    empty = emptyChild;
    if (atexit(removeSemaphore) != 0)
        errExit("atexit");
    
    struct sigaction sigAction;
    memset(&sigAction,0,sizeof(sigAction));
    sigAction.sa_handler = &signalHandler;
    if ((sigemptyset(&sigAction.sa_mask) == -1) || (sigaction(SIGINT,&sigAction, NULL) == -1))
        perror("Failed to install SIGINT signal handler");
    /*for (int i = 0; i < 10; i++)
    {
        
        memcpy(&mag, sharedMemoryAddr+i*sizeof(int), sizeof(int));
        //int mag = atoi(buf);
        printf("mag-> %d\n",mag);
    }
    mag = 99;
    memcpy(sharedMemoryAddr,&mag,sizeof(int));
    memcpy(&mag2, sharedMemoryAddr, sizeof(int));
    printf("====> %d\n",mag2);*/
    //int val=0;
    int idOfClient = 0;
    int sizeOfMatrix = 0;
    int **matrix = NULL;
    int * tempP;
    //printf("DISARDA:: %d\n",getpid());
    
    /*while(1){
        memcpy(&idOfClient, sharedMemoryAddr, sizeof(int));
        if(idOfClient == -1){
            
            sleep(5);
        }
        printf("MAIN: sharedMemoryAddr: %p\n",sharedMemoryAddr);

        printf("UYUYOR\n");
        fflush(stdout);
    }*/
    
    for(;;){
        if(terminate){
            return -1;
        }
        //perror("BLOCKLANMADAN HEMEN ONCE");
        int mutex_val = -45,emprtt=12;

        sem_getvalue(fullChild,&mutex_val);
        sem_getvalue(mutexChild,&emprtt);

        //printf("fullChild: %d",mutex_val);
        //printf("mutexChild: %d",emprtt);
        //fflush(stdout);
        // consumer will try to read
        sem_wait(fullChild);
        if(terminate){
            return -1;
        }
        sem_wait(mutexChild);
        if(terminate){
            return -1;
        }
        /*perror("ICERDEEEEEEEEEEEEEe");
        fflush(stderr);
        printf("ICERDE:: %d\n",getpid());
        fflush(stdout);*/
        tempP = (int*) sharedMemoryAddr;
        //sleep(10);
        // consume matrix
        /*memcpy(&val, sharedMemoryAddr, sizeof(int));
        if(val == -1){
            printf("SHARED MEMORY BOS\n");
        } else{
            printf("SHARED MEMORY BOS DEGIL:%d\n",val);

        }*/
                
            
        if(terminate){
            // its a child process, semaphores will be close and unlinked by parent
            return -1;
        }
        memcpy(&idOfClient, tempP, sizeof(int));

        tempP++;
        
        if(idOfClient == -1){
            perror("Something went wrong. Semaphores does not work correctly");
            return -1;            
        }
        memcpy(&sizeOfMatrix, tempP++, sizeof(int));
        struct flock lockToWrite;
        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printTime();
        printf("Z:Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy -1/%d\n",getpid(),idOfClient,sizeOfMatrix,sizeOfMatrix,poolsize);
        fflush(stdout);
        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        //int sizeSquare = sizeOfMatrix * sizeOfMatrix;
        matrix = (int**) malloc(sizeof(int*) * sizeOfMatrix);
        if(matrix == NULL){
            perror("malloc error");
            return -1;
        }
        // create space for matrix and take the matrix from shared space
        for (int i = 0; i < sizeOfMatrix; i++)
        {
            matrix[i] = (int*) malloc (sizeof(int) * sizeOfMatrix);
            if(matrix[i] == NULL){
                perror("Malloc error");
                for (int j = 0; j < i; j++)
                    free(matrix[j]);
                
                return -1;
            }
            for(int j = 0;j<sizeOfMatrix;j++){
                memcpy(&matrix[i][j], tempP++, sizeof(int));
            }
        }
        // tempP shows id of next client or it holds adderss of -1, which shows end of shared memory
        // taken data can be removed from shared memory
        // we will place the next matrix and its data in its position
        /*perror("BURDAYSA IYI GENE BASTI MI????");
        for(int i=0;i<sizeOfSharedMemory;i++){
            printf("%d ",((int*)sharedMemoryAddr)[i]);
        }
        printf("\n");*/
        /*perror("BUYUTME");
        sharedMemoryAddr = expandSharedSpace(sharedMemFd,0,sizeOfSharedMemory * 2); 
        for(int i=0;i<sizeOfSharedMemory;i++){
            printf("%d ",((int*)sharedMemoryAddr)[i]);
        }
        printf("\n");
        if(sharedMemoryAddr == NULL){
            shm_unlink(sharedMemory);
            perror("Share space could not be expanded");
            return -1;
        }*/
        //int * swapper = tempP;
        int * possibleMinusOneIndex= tempP;
        /*printf("TEMPP:%d\n",*tempP);
        fflush(stdout);*/
        int *to = (int*)sharedMemoryAddr;
        /*while(1){

            for(int* i=to;i<possibleMinusOneIndex;i++,swapper++){
                //printf("%d",*tempP);
                *i = *swapper;
            }
            if(*possibleMinusOneIndex == -1){
                break;
            } else{
                // possibleMinusOneIndex shows Id, there is at least one more matrix
                int size = possibleMinusOneIndex[1]; // [0] holds id of next matrix
                to = possibleMinusOneIndex; // next matrix will be placed of it.
                possibleMinusOneIndex = possibleMinusOneIndex[size+2]; //[id,size,0,1,2,...,32](-1|next id)
                swapper = possibleMinusOneIndex; // start replacing from index of id
            }
        }*/
        int * transferred = possibleMinusOneIndex;
        while(1){
            //perror("KAYBOLDU BURDA");
            while(to < possibleMinusOneIndex){
                *to = *transferred;
                to++;
                transferred++;

            }
            //to == possibleMinusOneIndex
            if(*possibleMinusOneIndex == -1){
                break;

            } else{
                int *idIndex = possibleMinusOneIndex;
                idIndex++;
                idIndex++;
                //idIndex Holds First Element Of matrix
                for(int k=0;k<sizeOfMatrix;k++){
                    idIndex++;
                }
                //idIndex += sizeOfNextMatrix;
                // now Id index holds possible - index
                possibleMinusOneIndex = idIndex;

            }
        }
        //while(1){
            /*if(*possibleMinusOneIndex != -1){
                while(swapper!=possibleMinusOneIndex){
                    *to = *swapper;
                    to++;
                    swapper++;
                }
                if(*swapper == -1)
            }*/
        //}
        /*perror("BURDAYSA IYI GENE");
        for(int i=0;i<sizeOfSharedMemory;i++){
            printf("%d ",((int*)sharedMemoryAddr)[i]);
        }*/
        
        //printf("\n");
        //fflush(stdout);
        /*memcpy(&val, tempP, sizeof(int));
        if(val == -1){
            printf("SON MATRIX : -1");
        } else{
            printf("ILK INDEx : %d",val);
        }*/
        // delete first matrix from shared space
        //perror("WORKERZ: ");
        /*for (int i = 0; i < sizeOfMatrix; i++)
        {
            for (int j = 0; j < sizeOfMatrix; j++)
            {
                printf("%d ",matrix[i][j]);
            }
            printf("\n");
        }*/
        if(terminate){
            for(int i=0;i<sizeOfMatrix;i++){
                free(matrix[i]);
            }
            free(matrix);

            return -1;
        }
        //printf("WORKER Z BASTI\n");
        //fflush(stdout);
        sem_post(mutexChild);
        sem_post(emptyChild);
        // send result to client
        snprintf(clientFifoResult, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_RESULT_TEMPLATE,(long) idOfClient);
        
        
        int clientResultFd;
        while((clientResultFd = open(clientFifoResult, O_WRONLY)) == -1 && errno == ENOENT );
        if (clientResultFd == -1){
            perror("open");
            return -1;
        }
        //char  str[] = "selam";
        
        int result = isInvertible(matrix,sizeOfMatrix);
        if(result){
            kill(getppid(),SIGUSR1); // increments number of invertible matrixes
        }
        //result = 4;
        // send process id to server to be able to create special fifo to send matrix
        //printf("id: %d clientFifoResult: %s\n",getpid(),clientFifoResult);
        if(terminate){
            for(int i=0;i<sizeOfMatrix;i++){
                free(matrix[i]);
            }
            free(matrix);
            return -1;
        }
        sleep(sleepDuration);
        //Z:Worker PID#800 responding to client PID#700: the matrix IS NOT invertible.
        //if(write(clientResultFd, &result, sizeof(int)) != sizeof(int))
        //struct flock lockToWrite;
        //memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printTime();
        printf("Z:Worker PID#%d responding to client PID#%d: the matrix %s invertible.\n",getpid(),idOfClient,(result) ? "is" : "IS NOT" );
        fflush(stdout);
        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite); 
        while((write(clientResultFd, &result, sizeof(int)) == -1 )&& errno == EINTR );
        
        for(int i=0;i<sizeOfMatrix;i++){
            free(matrix[i]);
        }
        free(matrix);

    }
    return 0;
}
