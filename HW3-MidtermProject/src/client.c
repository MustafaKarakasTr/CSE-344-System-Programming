/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 44-8 */


#include "client.h"
#include "linkedList/linkedList.h"

#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
#define CLIENT_FIFO_RESULT_TEMPLATE "/tmp/result_cl.%ld"
                                /* Template for building client FIFO name */
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
static char clientFifo[CLIENT_FIFO_NAME_LEN]; // to send matrix
static char clientFifoResult[CLIENT_FIFO_NAME_LEN]; // to take result from server

static void removeFifo(void)             /* Invoked on exit to delete client FIFO */
{
    unlink(clientFifo);

}
sig_atomic_t terminate = 0;
void signalHandler(int sig){
    terminate = 1;
}
/*
int main(){
    int size;
    int ** matrix = readMatrixFromFile("input.txt",&size);
    if(matrix == NULL){
        return -1;
    }
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");

        
    }
    freeMatrix(matrix,size);
}*/
void printTime(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
int main(int argc, char *argv[])
{
    int serverFd, clientFd,size,clientResultFd;


    if (argc != 5 || strcmp(argv[1], "-s") || strcmp(argv[3], "-o")){
        perror("./client -s pathToServerFifo -o pathToDataFile\n");
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
    //printf("%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    int ** matrix = readMatrixFromFile(argv[4],&size);
    if(matrix == NULL){
        return -1;
    }

    /* Create our FIFO (before sending request, to avoid a race) */

    umask(0);                   /* So we get the permissions we want */
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,
            (long) getpid());
    
    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1
                && errno != EEXIST)
    {
        perror("mkfifo");
        freeMatrix(matrix,size);
        return -1;
    }

    if (atexit(removeFifo) != 0)
    {
        perror("atexit");
        freeMatrix(matrix,size);
        return -1;
    }
    if(terminate){
        freeMatrix(matrix,size);
        return -1;
    }
    /* Construct request message, open server FIFO, and send message */

    /*req.pid = getpid();
    req.seqLen = (argc > 1) ? getInt(argv[1], GN_GT_0, "seq-len") : 1;
    */
    char * serverFifo = argv[2];
    //printf("%s\n",serverFifo);

    serverFd = open(serverFifo, O_WRONLY);
    if (serverFd == -1)
    {
        perror("open");
        freeMatrix(matrix,size);
        return -1;
    }
    //char  str[] = "selam";
    int id = getpid();

    // send process id to server to be able to create special fifo to send matrix
    if(write(serverFd, &id, sizeof(int)) != sizeof(int))
        fatal("Can't write to server");

    if (close(serverFd) == -1)
            errMsg("close");
    

    /* Open our FIFO, read and display response */

    clientFd = open(clientFifo, O_WRONLY);
    if (clientFd == -1)
    {
        perror("open");
        return -1;
    }
    double time_spent = 0.0;
 
    clock_t begin = clock();
    printf("Client PID#%d (%s) is submitting a %dx%d matrix\n",getpid(),argv[4],size,size);

    // first sents row size of matrix 
    if(write(clientFd, &size, sizeof(int)) != sizeof(int))
        fatal("Can't write to server");
    /*for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++){
            // sends array elements
            if(write(clientFd, &matrix[i][j], sizeof(int)) != sizeof(int))
                fatal("Can't write to server");
            printf("Yollandi: %d",i);
            fflush(stdout);
            sleep(1);
        }

        
        //sleep(1);
    }*/
    /*for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");

    }*/
    for(int k = 0;k<size;k++){
        if(write(clientFd, matrix[k], sizeof(int) * size) != (sizeof(int) * size ))
            fatal("Can't write to server");

    }
    /*for (int i = 0; i < 10; i++)
    {
        if(write(clientFd, &i, sizeof(int)) != sizeof(int))
                fatal("Can't write to server");
            printf("Yollandi: %d",i);
            fflush(stdout);
            sleep(1);
    }*/
    
    if (close(clientFd) == -1)
        errMsg("close");
    
    
    /*if (read(clientFd, &resp, sizeof(struct response))
            != sizeof(struct response))
        fatal("Can't read response from server");
    */
    //printf("%d\n", resp.seqNum);
    freeMatrix(matrix,size);

    if(terminate){
        return -1;
    }
    // take result
    snprintf(clientFifoResult, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_RESULT_TEMPLATE,
            (long) getpid());
    struct stat stats;
    if ( stat( clientFifoResult, &stats ) < 0 )
    {
        if ( errno != ENOENT )          // ENOENT is ok, since we intend to delete the file anyways
        {
            perror( "stat failed" );    // any other error is a problem
            return( -1 );
        }
    }
    else                                // stat succeeded, so the file exists
    {
        if ( unlink( clientFifoResult ) < 0 )   // attempt to delete the file
        {
            perror( "unlink failed" );  // the most likely error is EBUSY, indicating that some other process is using the file
            return( -1 );
        }
    }
    while (mkfifo(clientFifoResult, S_IRUSR | S_IWUSR | S_IWGRP) == -1
                && errno != EEXIST)
        printf("mkfifo %s", clientFifoResult);
    clientResultFd = open(clientFifoResult, O_RDONLY);
    if (clientResultFd == -1)
    {
        perror("open");
        return -1;
    }
    int result = 0;
    if (read(clientResultFd, &result, sizeof(int)) // reads result from server
            != sizeof(int)) {
        fprintf(stderr, "Error reading request; discarding\n");
                      /* Either partial read or error */
    }
    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    if(result)
        printf("Client PID#%d: the matrix is invertible, total time %f seconds, goodbye.\n",getpid(),time_spent);
    else
        printf("Client PID#%d: the matrix IS NOT invertible, total time %f seconds, goodbye.\n",getpid(),time_spent);

    unlink(clientFifo);
    unlink(clientFifoResult);
    return 0;
    //exit(EXIT_SUCCESS);
}
int** readMatrixFromFile(char *filename,int * size){
    int fd = open(filename,O_RDONLY);
    if(fd == -1){
        perror("Input file could not be opened");
        return NULL;
    }
    int ** matrix = NULL;
    char charRead;
    int bytesread;
    int numberOfElements = 0;
    int value = 0;
    char negative = 0; // bool value
    LinkedList * head = (LinkedList*) calloc(sizeof(LinkedList),1);
    if(head == NULL){
        perror("malloc error");
        return NULL;
    }
    head->next = NULL;
    char firstLine = 1;
    int rowSize = 0;
    //int x = 0,
    int y = 0;
    for(;;){
        //memset(buf,'\0',BLKSIZE);
        while(((bytesread = read(fd,&charRead,sizeof(char))) == -1) && (errno == EINTR)); // try to read until become successful
        //printf("ASCII: %d->%c ::: %d %d\n",charRead,charRead,bytesread,numberOfElements);
        if(bytesread == -1){
            perror("Input file could not be read");
            if(close(fd) == -1)
                perror("File could not be closed");
            return NULL;
        } else if(bytesread == 0){
            if(numberOfElements < rowSize * rowSize){

                matrix[rowSize-1][rowSize-1] = (negative) ? -value : value;
            }
            break;
        } else if(charRead == ',' || charRead == '\n'){ // int is ready
            
            if(negative){
                value = -value;
                negative = 0;
            }
            
            numberOfElements++;
            if(charRead == '\n' && matrix == NULL){ // first line is completed, copy the content of linked list
                firstLine = 0;
                rowSize = numberOfElements;
                
                matrix = (int**) malloc (sizeof(int*) * numberOfElements);
                if(matrix == NULL){
                    perror("malloc error");
                    if(close(fd) == -1)
                        perror("File could not be closed");
                    return NULL;
                }
                
                for(int i=0;i<numberOfElements;i++){
                    matrix[i] = (int*) malloc(sizeof(int) * numberOfElements);
                    if(matrix[i] == NULL){
                        perror("malloc error");
                        // free allocated spaces so far
                        for(int j=0;j<i;j++){
                            free(matrix[j]);
                        }
                        free(matrix);
                        if(close(fd) == -1)
                            perror("File could not be closed");
                        return NULL;
                    }
                }
                LinkedList * temp = head;
                for(int i=0;i<rowSize-1;i++,temp = temp->next){
                    matrix[0][i] = *((int*)temp->data); // cast from void * to int
                    free(temp->data);
                }
                matrix[0][rowSize-1] = value;
                freeLinkedList(head);
                value = 0;
                
            } else if(firstLine){ // first line is being read, save to linked list
                int * element = (int*) malloc(sizeof(int));
                
                if(element == NULL){ 

                    perror("malloc error");
                    // matrix is not allocated yet, no need to free it
                    if(close(fd) == -1)
                        perror("File could not be closed");
                    return NULL;
                }
                *element = value;
                //printf("value: %d\n",*element);
                value = 0;
                 //int success;
                //(head->data == NULL) ? head->data = element: (success = add(head,element));
                if(head->data == NULL){
                    head->data = element;
                } else{
                    add(head,element);
                }
                //LinkedList * t = head;
                /*for(;t!=NULL && t->data != NULL;){
                    printf("%d->",*((int*)t->data));
                    t = t->next;
                }*/
                /*if(success == -1){
                    if(close(fd) == -1)
                        perror("File could not be closed");
                    return NULL;
                }*/
            } else { // int value which is not in first line has been read
                //printf("VAL: %d\n",value);
                matrix[(numberOfElements - 1) / rowSize][(numberOfElements -1) % rowSize ] = value;
                value = 0;
                if(y == rowSize){
                    y = 0;
                }
            }
            if(bytesread == 0)
                break;
        } else if(charRead == '-'){
            if(value != 0){
                perror("'-' character is in invalid position in input file");
                freeMatrix(matrix,rowSize);
                if(close(fd) == -1)
                        perror("File could not be closed");
                return NULL;
            }
            negative = 1;
        } else if(charRead < '0' || charRead > '9'){
            perror("Input file has invalid character");
            freeMatrix(matrix,rowSize);
            if(close(fd) == -1)
                perror("File could not be closed");
            return NULL;
        }  
        else { // numeric value is read
            value *= 10;
            value += (charRead - '0');
        }
        
    }
    if(close(fd) == -1){
        perror("File could not be closed");
        freeMatrix(matrix,rowSize);
    }
    *size = rowSize;
    return matrix;    
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
