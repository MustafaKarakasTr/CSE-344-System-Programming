//#include "../header/helper.h"
// matrix includes helper
#include "../header/matrix.h"
#define BUFSIZE 500
int * createIds(int size,int startId){
    int * ids = (int*) malloc(sizeof(int) * size);
    if(ids == NULL){
        perror("malloc");
        return NULL;
    }
    for (int i = 0; i < size; i++)
    {
        ids[i] = startId++;
    }
    return ids;
} 
/*
typedef struct threadArgs{
    int idOfThread;
    int ** matrix1;
    int ** matrix2;
    int size;
    int startColumn;
    int endColumn;

};
*/

threadArgs * createArguments(int ** matrix1,int ** matrix2,int ** ans_matrix,ImaginaryNumber ** fourier_matrix,int size,int numberOfThreads,int* IDs){
    threadArgs * args = (threadArgs *) malloc(sizeof(threadArgs) * numberOfThreads);
    if(args == NULL){
        return NULL;
    }
    int numberOfColumnsPerThread = size / numberOfThreads;
    int divisiblityCheck = numberOfColumnsPerThread * numberOfThreads;
    int numberOfThreadsWhichTakesMoreColumn = 0;
    if(divisiblityCheck == size){
        // every thread takes equal number of columns as a task
    } else{
        numberOfThreadsWhichTakesMoreColumn = size - divisiblityCheck;
        //printf("numberOfThreadsWhichTakesMoreColumn: %d\n",numberOfThreadsWhichTakesMoreColumn);
    }
    int startIndexOfColumn = 0;
    for (size_t i = 0; i < numberOfThreads; i++)
    {
        args[i].matrix1 = matrix1;
        args[i].matrix2 = matrix2;
        args[i].ans_matrix = ans_matrix;
        args[i].size = size;
        args[i].idOfThread = IDs[i];
        args[i].startColumn = startIndexOfColumn;
        startIndexOfColumn += numberOfColumnsPerThread;
        if(i < numberOfThreadsWhichTakesMoreColumn){
            startIndexOfColumn++;
        }        
        args[i].endColumn = startIndexOfColumn;
        args[i].numberOfThreads = numberOfThreads;
        args[i].discreteFourierTransformOfAns_Matrix = fourier_matrix;

    }
    return args;

}
pthread_t* createThreads(int numberOfThreads,void *(*start_routine) (void *),threadArgs * args){
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)* numberOfThreads);
    if(threads == NULL){
        return NULL;
    }

    for (int i = 0; i < numberOfThreads; i++)
    {
        if(pthread_create(&threads[i],NULL,start_routine,&args[i]) != 0){
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
void joinThreads(pthread_t * threads,int size){
    for (int i = 0; i < size; i++)
    {
        if(pthread_join(threads[i],NULL) != 0){
            perror("pthread_join");
            // do not end the program
        }

    }

}
int writeTransform(const char* fileName,ImaginaryNumber ** matrix,int size){
    int fd = open(fileName,O_CREAT|O_TRUNC|O_WRONLY,S_IRWXU | S_IRWXG);
    if (fd == -1){
        perror("open");
        return -1;        
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    fcntl(fd,F_SETLKW,&lock);

    char buf[BUFSIZE];
    char format[100] = "%f+j(%f),";
    int byteswritten = 0;
    //printMatrixImaginaryNumber(discreteFourierTransformOfAns_Matrix,size);
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            snprintf(buf,BUFSIZE,format,matrix[i][j].real, matrix[i][j].imaginary);
            
            while(((byteswritten = write(fd,buf,strlen(buf) - (j == size-1) )) == -1 )&& errno == EINTR );

        }
        while((byteswritten = write(fd,"\n",1) == -1 )&& errno == EINTR );
    }
    if(close(fd) == -1){
        perror("close");
        return -1;
    }
    return 0;

}