#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include "../header/matrix.h"
#include "../header/helper.h"

#define BUFFERSIZE 100

struct timeval initialTime;
sig_atomic_t terminate = 0;
pthread_cond_t cond;
pthread_mutex_t mtx;
int arrived = 0;
void signalHandler(int sig){
    terminate = 1;    
}
void * func(void *p){
    struct timeval now,lastTime;

    threadArgs * arg = (threadArgs*)p;
    subSquareMatrixMultiplication(arg->ans_matrix,arg->matrix1,arg->matrix2,arg->size,arg->startColumn,arg->endColumn);
    if(terminate){
        perror("TERMINATE");
        return NULL;
    }
    if (gettimeofday(&now, NULL) == -1){
        perror("gettimeofday");
        return NULL;
    }
    long secondsPassed = (long) now.tv_sec - initialTime.tv_sec;
    long microsecondsPassed = (long) now.tv_usec - initialTime.tv_usec;
    char buf[BUFFERSIZE];
    /*
        1 microsecond = 1.0 × 10-6 seconds
        seconds: %ld microseconds: %ld 
    */
    double microsecondsToSeconds = ((double)microsecondsPassed) / 1000000.0;
    
    char formatRead[] = "Seconds: %ld Microseconds: %ld Thread %d has reached the rendezvous point in %f seconds\n";
    
    if(snprintf(buf,BUFFERSIZE,formatRead,secondsPassed,microsecondsPassed,arg->idOfThread,secondsPassed+microsecondsToSeconds) < 0){
        perror("snprintf consumed");
        return NULL;
    }
    if(terminate){
        return NULL;
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    // first part ends
    pthread_mutex_lock(&mtx);
    ++arrived;
    char lastOne = 1;
    while(arrived < arg->numberOfThreads){
        //printf("Thread %d,sonlandi bekliyor\n",arg->idOfThread);
        pthread_cond_wait(&cond,&mtx);
        lastOne = 0;
    }
    if(lastOne){
        pthread_cond_broadcast(&cond);
    }
    // Thread 1 is advancing to the second part

    //printf("Thread %d,devam ediyor\n",arg->idOfThread);
    pthread_mutex_unlock(&mtx);
    if(terminate){
        return NULL;
    }
    char formatAdvancing[] = "Seconds: %ld Microseconds: %ld Thread %d is advancing to the second part\n";
    if (gettimeofday(&now, NULL) == -1){
        perror("gettimeofday");
        return NULL;
    }
    secondsPassed = (long) now.tv_sec - initialTime.tv_sec;
    microsecondsPassed = (long) now.tv_usec - initialTime.tv_usec;
    if(snprintf(buf,BUFFERSIZE,formatAdvancing,secondsPassed,microsecondsPassed,arg->idOfThread) < 0){
        perror("snprintf consumed");
        return NULL;
    }
    if(terminate){
        return NULL;
    }
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    if(terminate){
        return NULL;
    }
    subSquareMatrix2DDiscreteFourierTransform(arg->discreteFourierTransformOfAns_Matrix,arg->ans_matrix,arg->startColumn,arg->endColumn,arg->size);
    if(terminate){
        return NULL;
    }
    //Thread 1 has has finished the second part in 0.1321 seconds.
    if (gettimeofday(&lastTime, NULL) == -1){
        perror("gettimeofday");
        return NULL;
    }
    char formatFinished[] = "Seconds: %ld Microseconds: %ld\tThread %d has finished the second part in %f seconds.\n";
    secondsPassed = (long) lastTime.tv_sec - now.tv_sec;
    microsecondsPassed = (long) lastTime.tv_usec - now.tv_usec;
    microsecondsToSeconds = ((double)microsecondsPassed) / 1000000.0;
    long totalTimePassedSeconds = (long) lastTime.tv_sec - initialTime.tv_sec;
    long totalTimePassedMicroseconds = (long) lastTime.tv_usec - initialTime.tv_usec;
    double totalSecondsPassed = microsecondsToSeconds + secondsPassed; 
    if(snprintf(buf,BUFFERSIZE,formatFinished,totalTimePassedSeconds,totalTimePassedMicroseconds,arg->idOfThread,totalSecondsPassed) < 0){
        perror("snprintf consumed");
        return NULL;
    }
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    
    return NULL;
}

int int_pow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp % 2)
           result *= base;
        exp /= 2;
        base *= base;
    }
    return result;
}
// ./hw5 -i filePath1 -j filePath2 -o output -n 4 -m 2


int main(int argc, char const *argv[])
{
    if(argc != 11){
        perror("Arguments are not correct.\nPlease enter arguments in the format: ./hw5 -i filePath1 -j filePath2 -o output -n 4 -m 2\n");
        return -1;
    }
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &signalHandler;
    if ((sigemptyset(&sa.sa_mask) == -1) ||  (sigaction(SIGINT,&sa, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    const char * inFile1 = argv[2];
    const char * inFile2 = argv[4];
    const char * outFile = argv[6];
    int n = atoi(argv[8]);
    if(n <= 2){
        perror("n must be greater than 2");
        return -1;
    }
    int m = atoi(argv[10]);
    if(m < 2){
        perror("n must be greater than or equal to 2");
        return -1;
    }

    //printf("Args good %s %s %s %2d %3d\n",inFile1,inFile2,outFile ,n,m);
    // allocate matrixes :  It will read sequentially (2^n)x(2^n) characters
    int twoToTheN = int_pow(2,n);
    int **matrix1 = allocateMatrix(twoToTheN);
    struct timeval now,startsWhenReadToMem;
    if (gettimeofday(&initialTime, NULL) == -1){
        perror("gettimeofday");
        return -1;
    }
    if(matrix1 == NULL){
        perror("matrix can not be allocated");
        return -1;
    }
    int **matrix2 = allocateMatrix(twoToTheN);
    if(matrix2 == NULL){
        freeMatrix((void**)matrix1,twoToTheN);
        perror("matrix can not be allocated");
        return -1;
    }
    
    //printf("\n");

    if(readMatrixFromFile(matrix1,twoToTheN,inFile1) == -1 || terminate || readMatrixFromFile(matrix2,twoToTheN,inFile2) == -1 || terminate){
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        return -1;
    }
    ;
    if (gettimeofday(&startsWhenReadToMem, NULL) == -1){
        perror("gettimeofday");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        return -1;
    }
    char buf[BUFFERSIZE];
    char formatRead[] = "Two matrices of size %dx%d have been read. The number of threads is %d\n";
    if(snprintf(buf,BUFFERSIZE,formatRead,twoToTheN,twoToTheN,m) < 0){
        perror("snprintf consumed");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        return -1;
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));

    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock); 

    /*printMatrix(matrix1,twoToTheN);
    printf("\n");    
    printMatrix(matrix2,twoToTheN);*/
    int ** ans_matrix = allocateMatrix(twoToTheN);
    if(ans_matrix == NULL || terminate){
        if(!terminate)
            perror("matrix can not be allocated");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        return -1;
    }
    ImaginaryNumber ** discreteFourierTransformOfAns_Matrix = allocateMatrixImaginaryNumber(twoToTheN);
    if(discreteFourierTransformOfAns_Matrix == NULL || terminate){
        if(!terminate)
            perror("matrix can not be allocated");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        
        return -1;
    }
    /*int ** matrix = subSquareMatrixMultiplication(ans_matrix,matrix1,matrix2,twoToTheN,0,twoToTheN);
    printMatrix(matrix,twoToTheN);
    return 0;*/
    // create mutexes
    pthread_mutexattr_t mtxAttr;
    int s;
    s = pthread_mutexattr_init(&mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_init");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);

        return -1;
    }
    s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_ERRORCHECK);
    if (s != 0){
        perror("pthread_mutexattr_settype");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);

        return -1;
    }
    s = pthread_mutex_init(&mtx, &mtxAttr);
    if (s != 0){
        perror("pthread_mutex_init");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);

        return -1;
    }
    if(pthread_cond_init(&cond, NULL) != 0){
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);
        s = pthread_mutexattr_destroy(&mtxAttr); /* No longer needed */
        if (s != 0)
            perror("pthread_mutexattr_destroy");
        return -1;
    }
    int * IDs = createIds(m,1);
    if(IDs == NULL || terminate){
        if(!terminate)
            perror("IDs can not be allocated");
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);
        s = pthread_mutexattr_destroy(&mtxAttr); /* No longer needed */
        if (s != 0)
            perror("pthread_mutexattr_destroy");
        if(pthread_cond_destroy(&cond))
            perror("pthread_cond_destroy");
        return -1;
    }
    threadArgs * threadArguments = createArguments(matrix1,matrix2,ans_matrix,discreteFourierTransformOfAns_Matrix,twoToTheN,m,IDs); 
    if(threadArguments == NULL || terminate){
        if(!terminate)
            perror("threadArguments could not be created");
        free(IDs);
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);
        s = pthread_mutexattr_destroy(&mtxAttr); /* No longer needed */
        if (s != 0)
            perror("pthread_mutexattr_destroy");
        if(pthread_cond_destroy(&cond))
            perror("pthread_cond_destroy");
        return -1;
    }

    
    pthread_t * threads = createThreads(m,func,threadArguments);
    if(threads == NULL){
        perror("threads could not be created");
        free(IDs);
        free(threadArguments);
        freeMatrix((void**)matrix1,twoToTheN);
        freeMatrix((void**)matrix2,twoToTheN);
        freeMatrix((void**)ans_matrix,twoToTheN);
        freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);

        s = pthread_mutexattr_destroy(&mtxAttr); /* No longer needed */
        if (s != 0)
            perror("pthread_mutexattr_destroy");
        if(pthread_cond_destroy(&cond))
            perror("pthread_cond_destroy");
        return -1;
    }
    joinThreads(threads,m);
    if(terminate){
        // do not enter other blocks
    } else if (gettimeofday(&now, NULL) == -1){
        perror("gettimeofday");
        // no need to end the program, 'now' object will not used outside of else statement
    } else {
        long secondsPassed = (long) now.tv_sec - startsWhenReadToMem.tv_sec;
        long microsecondsPassed = (long) now.tv_usec - startsWhenReadToMem.tv_usec;
        double microsecondsToSeconds = ((double)microsecondsPassed) / 1000000.0;
        double totalSeconds = microsecondsToSeconds + (double)secondsPassed;
        // total time
        char totalTimeFormat[] = "Total time spent, from the moment the files were read into memory, until the calculations were completed:%f seconds\n";
        char buf2[500];
        if(snprintf(buf2,500,totalTimeFormat,totalSeconds) < 0){
            perror("snprintf consumed");
            /*freeMatrix((void**)matrix1,twoToTheN);
            freeMatrix((void**)matrix2,twoToTheN);
            return -1;*/
        } else{
            memset(&lock,0,sizeof(lock));
            lock.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
            while((write(STDOUT_FILENO,buf2,strlen(buf2)) == -1) && errno == EINTR);
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        }
        //printf("Total time spent, from the moment the files were read into memory, until the calculations were completed:%f seconds\n",totalSeconds);
    }
    if(!terminate)
        writeTransform(outFile, discreteFourierTransformOfAns_Matrix, twoToTheN);
    //The process has written the output file. The total time spent is 0.173 seconds.
    char formatWritten[] = "Seconds: %ld Microseconds: %ld The process has written the output file. The total time spent is %f seconds.\n"; 
    if(terminate){
        // do not enter other blocks
    } else if (gettimeofday(&now, NULL) == -1){
        perror("gettimeofday");
        
    } else{
        long secondsPassed = (long) now.tv_sec - initialTime.tv_sec;
        long microsecondsPassed = (long) now.tv_usec - initialTime.tv_usec;
        //char buf[BUFFERSIZE];
        /*
            1 microsecond = 1.0 × 10-6 seconds
        */
        char buf2[500];
        double microsecondsToSeconds = ((double)microsecondsPassed) / 1000000.0;
        double totalSeconds = microsecondsToSeconds + (double)secondsPassed;
        if(snprintf(buf2,500,formatWritten,secondsPassed,microsecondsPassed,totalSeconds) < 0){
            perror("snprintf consumed");
            /*freeMatrix((void**)matrix1,twoToTheN);
            freeMatrix((void**)matrix2,twoToTheN);
            return -1;*/
        } else{
            memset(&lock,0,sizeof(lock));
            lock.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
            while((write(STDOUT_FILENO,buf2,strlen(buf2)) == -1) && errno == EINTR);
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        }
    }
    s = pthread_mutexattr_destroy(&mtxAttr); /* No longer needed */
    if (s != 0)
        perror("pthread_mutexattr_destroy");
    if(pthread_cond_destroy(&cond))
        perror("pthread_cond_destroy");
    free(IDs);
    free(threadArguments);
    free(threads);
    freeMatrix((void**)matrix1,twoToTheN);
    freeMatrix((void**)matrix2,twoToTheN);
    freeMatrix((void**)ans_matrix,twoToTheN);
    freeMatrix((void**)discreteFourierTransformOfAns_Matrix,twoToTheN);
    return 0;
}
