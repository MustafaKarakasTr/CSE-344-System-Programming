#include "../header/threadHelper.h"
#include "../header/client.h"

pthread_t* createThreads(int numberOfThreads,void *(*start_routine) (void *), ClientThreadArg* args){
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)* numberOfThreads);
    if(threads == NULL){
        return NULL;
    }

    for (int i = 0; i < numberOfThreads; i++)
    {
        if(pthread_create(&threads[i],NULL,start_routine,(void*)&args[i]) != 0){
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