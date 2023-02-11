#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include <fcntl.h>
#include <signal.h>

#include <arpa/inet.h>

#include "../header/threadHelper.h"

#include "../header/client.h"

sig_atomic_t terminate = 0;
void signalHandler(int sig){
    terminate = 1;    
}
int portNumber=0;
char const * IP;
int barrierForThreads = 0;
pthread_cond_t cond;
pthread_mutex_t mtx;
int arrived = 0;
#define CONSTANT_STR "transactionCount"
//./client -r requestFile -q PORT -s IP
/*
    argv[0] filename
    argv[2] requestFile
    argv[4] portno
    argv[6] server_ipaddress
*/

int main(int argc , char* argv[]){

    if(argc != 7){
        fprintf(stderr,"usage %s -r requestFile -q PORT -s IP\n",argv[0]);
        exit(1);
    }
    portNumber = atoi(argv[4]);
    IP = argv[6];
    // sig handler
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &signalHandler;
    if ((sigemptyset(&sa.sa_mask) == -1) ||  (sigaction(SIGINT,&sa, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    int numberOfRequests = getNumberOfLines(argv[2]);
    barrierForThreads = numberOfRequests;
    char ** requests = (char **) malloc(numberOfRequests * sizeof(char *));
    if(requests == NULL){
        perror("Malloc error");
        return -1;
    }
    //perror("5");
    for (int i = 0; i < numberOfRequests; i++)
    {
        char * temp = (char*) malloc(BLKSIZE * sizeof(char));
        if(temp == NULL){
            perror("Malloc error");
            // free allocated areas so far
            for (int j = 0; j < i; j++)
            {
                free(requests[j]);
            }
            free(requests);
            return -1;
            
        }
        requests[i] = temp;
    }
    if(terminate){
        for (int i = 0; i < numberOfRequests; i++)
        {
            free(requests[i]);
        }
        free(requests);
        return 0;
    }
    //perror("4");
    int fd = open(argv[2],O_RDONLY);
    if(fd == -1){
        perror("open file error");
        return -1;
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    int bytesread=0;
    // read requests
    //perror("3");

    for(int reqNum = 0;reqNum < numberOfRequests;reqNum++){
    //for(;;){
        //printf("::%d %d ",reqNum,numberOfRequests);
        /*if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }*/
        memset(requests[reqNum],'\0',BLKSIZE);

        lock.l_type = F_RDLCK;
        /*Place a read lock on the file. */
        fcntl(fd,F_SETLKW,&lock);

        while(((bytesread = read(fd,requests[reqNum],BLKSIZE)) == -1) && (errno == EINTR)); // try to read until become successful
        
        lock.l_type = F_UNLCK;
        fcntl(fd,F_SETLKW,&lock);
        if(terminate){
            for (int i = 0; i < numberOfRequests; i++){
                free(requests[i]);
            }
            free(requests);
            if(close(fd) == -1){
                perror("close file error");
                return -1;
            }
            return 0;
        }
        /*if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }*/

        
        if(bytesread <=0) // end of file is reached 
        {
            break;
        }
        /*
            // put descriptor to start of newline
        */
        for (int i = 0; requests[reqNum][i] != '\0'; i++)
        {
            if(requests[reqNum][i] == '\n'){
                int howManyCharactersReadAfterNewLine = bytesread - i - 1;
                
                if(howManyCharactersReadAfterNewLine != 0){
                    lseek(fd,-howManyCharactersReadAfterNewLine,SEEK_CUR); 
                }
                requests[reqNum][i] = '\0';
                break;
            }
            //printf("LINE::: %s",buf);
        }
        int len = strlen(requests[reqNum]);
        if(len <= strlen(CONSTANT_STR))
            break;
        //takeTransactionInfoFromLine(buf,&transactionFile->transactions[index]);

    }
    if(close(fd) == -1){
        perror("close file error");
        return -1;
    }
    // create thread arguments
    ClientThreadArg * threadArgs = createThreadArgs(requests,numberOfRequests);
    if(threadArgs == NULL){
        perror("threadArgs could not be created");
        for (int i = 0; i < numberOfRequests; i++)
        {
            free(requests[i]);
        }
        free(requests);
        return -1;
    }
    // create threads
    char buf[100];
    char format[]="Client: I have loaded %d requests and I’m creating %d threads.\n";
    if(snprintf(buf,100,format,numberOfRequests,numberOfRequests) < 0){
        perror("snprintf consumed");
        return -1;
    }

    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);

    pthread_t * threads = createThreads(numberOfRequests,threadFunc,threadArgs); 
    if(threads == NULL){
        perror("threads could not be created");
        for (int i = 0; i < numberOfRequests; i++)
        {
            free(requests[i]);
        }
        free(requests);
        free(threadArgs);
        return -1;
    }
    joinThreads(threads,numberOfRequests);

    char buf2[] = "Client: All threads have terminated, goodbye.\n";
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf2,strlen(buf2)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    /*
    int sockfd , newsockfd, port_no , n;
    char buffer[255];
    
    struct sockaddr_in serv_addr;
    struct hostent *server;
    perror("2");

    port_no = atoi(argv[4]);

    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if(sockfd < 0){
        perror("error opening socket\n");
        exit(1); 
    }

    server = gethostbyname(argv[6]);

    if(server == NULL){
        perror("error no such host\n");
        exit(1);
    }
    perror("1");

    //bzero((char*) &serv_addr , sizeof(serv_addr));
    memset((char*) &serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    memcpy((char*) server->h_addr,(char *) &serv_addr.sin_addr.s_addr, server->h_length);
    bcopy((char*) server->h_addr , (char *) &serv_addr.sin_addr.s_addr , server->h_length);
    serv_addr.sin_port = htons(port_no);
    perror("Connectten once");
    
    while(connect(sockfd , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) == -1){
        close(sockfd);
        if(terminate)
            break;
        sockfd = socket(AF_INET , SOCK_STREAM , 0);
    }
    perror("Connectten sonra");
    if(terminate){
        perror("TERMINATEEEE");

        close(sockfd);
    
        for (int i = 0; i < numberOfRequests; i++)
        {
            free(requests[i]);
        }
        free(requests);
        return 0;
    }
    //while (1)
    for(int i=0;i<numberOfRequests;i++)
    {
        //memset(buffer,0,255);
        //fgets(buffer,255,stdin);
        while((n=write(sockfd , requests[i],BLKSIZE) == -1 ) && errno == EINTR);
        printf("YAZILDI: %s\n",requests[i]);

        if(n < 0){
            perror("error on writing");
            exit(1);
        }
        // int i = strcmp("Bye",buffer);
        // if(i==0){break;}
        // for (int i = 0; i < strlen(buffer); i++)
        // {
        //     printf("qq: %d %c\n",buffer[i],buffer[i]);
        // }
        // printf("\n");
    }
    
    close(sockfd);*/
    
    for (int i = 0; i < numberOfRequests; i++)
    {
        free(requests[i]);
    }
    free(requests);
    free(threads);
    free(threadArgs);
    return 0;
}
int getNumberOfLines(char const * fileName){
    struct flock lock;
    int fd = open(fileName,O_RDONLY);
    if(fd == -1){
        perror("open file error");
        return -1;
    }
    memset(&lock,0,sizeof(lock));
    
    char buf[BLKSIZE];
    int bytesread=0;
    // read requests
    int numberOfLines = 0;
    for(;;){
        /*if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }*/
        memset(buf,'\0',BLKSIZE);

        lock.l_type = F_RDLCK;
        /*Place a read lock on the file. */
        fcntl(fd,F_SETLKW,&lock);

        while(((bytesread = read(fd,buf,BLKSIZE)) == -1) && (errno == EINTR)); // try to read until become successful
        
        lock.l_type = F_UNLCK;
        fcntl(fd,F_SETLKW,&lock);
        /*if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }*/

        
        if(bytesread <=0) // end of file is reached 
        {
            break;
        }
        /*
            // put descriptor to start of newline
        */
        for (int i = 0; buf[i] != '\0'; i++)
        {
            if(buf[i] == '\n'){
                int howManyCharactersReadAfterNewLine = bytesread - i - 1;
                
                if(howManyCharactersReadAfterNewLine != 0){
                    lseek(fd,-howManyCharactersReadAfterNewLine,SEEK_CUR); 
                }
                buf[i] = '\0';
                break;
            }
            //printf("LINE::: %s",buf);
        }
        int len = strlen(buf);
        if(len > strlen(CONSTANT_STR))
            numberOfLines++;
        else 
            break;
        //takeTransactionInfoFromLine(buf,&transactionFile->transactions[index]);

    }
    if(close(fd))
        perror("Close error");
    
    return numberOfLines;
}
void * threadFunc(void *arg){
    //char buf[100];
    char buf[]="Client-Thread-: Thread- has been created\n";
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    
    ClientThreadArg* request = (ClientThreadArg*) arg;
    //printf("Thread: ARGS:%s %d/%d/%d %d/%d/%d SEHIR:%s\n",request->type,request->start_date.day,request->start_date.month,request->start_date.year,request->end_date.day,request->end_date.month,request->end_date.year,request->city);
    pthread_mutex_lock(&mtx);
    ++arrived;
    char lastOne = 1;
    while(arrived < barrierForThreads ){
        //printf("Thread %d,sonlandi bekliyor\n",arg->idOfThread);
        pthread_cond_wait(&cond,&mtx);
        lastOne = 0;
    }
    if(lastOne){
        pthread_cond_broadcast(&cond);
    }
    // Threads is advancing to the second part
    //printf("Thread %d,devam ediyor\n",arg->idOfThread);
    pthread_mutex_unlock(&mtx);

    //printf("Burda server ile iletişim kurulcak requestler yollancak\n");
    sendRequestToServer(request);
    //printf("Thread fonksiyonu: %s\n",request);
    /*char const * separator = " ";
    char * transactionCount;
    char * type;
    char * start_date_str;
    char * end_date_str;

    transactionCount = strtok(request, separator);
    type = strtok(NULL, separator);
    start_date_str = strtok(NULL, separator);
    end_date_str = strtok(NULL, separator);
    printf("%s type: %s %s %s\n",transactionCount,type,start_date_str,end_date_str);
    */
    /* walk through other tokens */
    /*for (int i = 0; i<5 && token!= NULL; i++)
    {
        if(token == NULL)
            return -1;
        //startEndIndexes[i] = atoi(token);
        //printf("%s-------\n",token);
        if(saveToTransaction(tr,token,i) == -1){
            return -1;
        }

        token = strtok(NULL, separator);
    }*/
    return NULL;
}
ClientThreadArg *createThreadArgs(char ** requests,int numberOfRequests){
    ClientThreadArg * args = (ClientThreadArg *) malloc(numberOfRequests * sizeof(ClientThreadArg));
    if(args == NULL){
        perror("Malloc error create thread args");
        return NULL;
    }
    for (int i = 0; i < numberOfRequests; i++)
    {
        //printf("Thread fonksiyonu: %s\n",request);
        char const * separator = " ";
        char * type;
        char * start_date_str;
        char * end_date_str;
        char * city;


        type = strtok(requests[i], separator);
        type = strtok(NULL, separator);
        strcpy(args[i].type,type);
        //args[i].type = type;
        start_date_str = strtok(NULL, separator);
        end_date_str = strtok(NULL, separator);
        
        city = strtok(NULL, separator);
        if(city != NULL){
            strcpy(args[i].city,city);    
        } else{
            args[i].city[0] = '\0';
        }
        //args[i].city = city;

        
        initializeDate(&(args[i].start_date),start_date_str);
        initializeDate(&(args[i].end_date),end_date_str);

    }
    return args;
    
}
int sendRequestToServer(ClientThreadArg * message){
    /*int sockfd , newsockfd, port_no , n;
    char buffer[255];
    
    struct sockaddr_in serv_addr;
    struct hostent *server;
    perror("2");

    port_no = portNumber;

    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if(sockfd < 0){
        perror("error opening socket\n");
        exit(1); 
    }
    perror("8666");
    fprintf(stderr, "IP: %s\n",IP);
    server = gethostbyname(IP);

    if(server == NULL){
        perror("error no such host\n");
        exit(1);
    }
    perror("897");*/
/*
    //bzero((char*) &serv_addr , sizeof(serv_addr));
    memset((char*) &serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    perror("!!!");
    memcpy((char *) &serv_addr.sin_addr.s_addr,(char*) server->h_addr, server->h_length);
    bcopy((char*) server->h_addr , (char *) &serv_addr.sin_addr.s_addr , server->h_length);
    perror("===");
    
    serv_addr.sin_port = htons(port_no);
    perror("66116");
    */
   /*
    bzero((char*) &serv_addr , sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    bcopy((char*) server->h_addr , (char *) &serv_addr.sin_addr.s_addr , server->h_length);
    serv_addr.sin_port = htons(port_no);*/
    struct sockaddr_in serv_addr;
    //char buffer[BUF_SIZE];
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket");
        return -1;
    }


    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNumber);

    inet_pton(AF_INET, IP, &serv_addr.sin_addr);

    while(connect(sockfd , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) == -1){
        close(sockfd);
        if(terminate)
            break;
        sockfd = socket(AF_INET , SOCK_STREAM , 0);
    }
    //perror("77777");

    if(terminate){
        //perror("TERMINATEEEE");

        close(sockfd);
    
        return 0;
    }
    //while (1)
    /*for(int i=0;i<numberOfRequests;i++)
    {
        //memset(buffer,0,255);
        //fgets(buffer,255,stdin);
        while((n=write(sockfd , requests[i],BLKSIZE) == -1 ) && errno == EINTR);
        printf("YAZILDI: %s\n",requests[i]);

        if(n < 0){
            perror("error on writing");
            exit(1);
        }
        // int i = strcmp("Bye",buffer);
        // if(i==0){break;}
        // for (int i = 0; i < strlen(buffer); i++)
        // {
        //     printf("qq: %d %c\n",buffer[i],buffer[i]);
        // }
        // printf("\n");
    }*/
    int n;
    /*struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;*/
    //while((n=write(sockfd , &bit,sizeof(u_int8_t)) == -1 ) && errno == EINTR);
    /*char buf[100];
    char format[]="Client-Thread-5: I am requesting \n";
    if(snprintf(buf,100,format,numberOfRequests,numberOfRequests) < 0){
        perror("snprintf consumed");
        return -1;
    }*/
    char format[] = "Client-Thread-: I am requesting 'transactionCount %s %d-%d-%d %d-%d-%d %s'\n";
    char buf[200];
    if(snprintf(buf,200,format,message->type,message->start_date.day,message->start_date.month,message->start_date.year,message->end_date.day,message->end_date.month,message->end_date.year,message->city) < 0){
        perror("snprintf consumed");
        return -1;
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    //printTime();
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    
    while((n=write(sockfd , message,sizeof(ClientThreadArg)) == -1 ) && errno == EINTR);
    int answer = 0;
    while((n=read(sockfd , &answer,sizeof(int)) == -1 ) && errno == EINTR);
    char format2[] = "Client-Thread- :The server’s response to 'transactionCount %s %d-%d-%d %d-%d-%d %s' is %d\nClient-Thread-: Terminating\n";
    char buf3[300];
    if(snprintf(buf3,300,format2,message->type,message->start_date.day,message->start_date.month,message->start_date.year,message->end_date.day,message->end_date.month,message->end_date.year,message->city,answer) < 0){
        perror("snprintf consumed");
        return -1;
    }
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    //printTime();
    while((write(STDOUT_FILENO,buf3,strlen(buf3)) == -1) && errno == EINTR);
    if(answer == -1){
        char errorBuf[]="Client-Thread- City does not exist\n";
        while((write(STDOUT_FILENO,errorBuf,strlen(errorBuf)) == -1) && errno == EINTR);

    }
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    //printf("CLIENT: %s : %d\n",(message->city[0] == 0) ? "ALL" : message->city,answer);
    /*lock.l_type = F_UNLCK;
    fcntl(sockfd,F_SETLKW,&lock);*/
    
    close(sockfd);
    return 0;
}