#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include <pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include "../header/server.h"

#include "../header/client.h"

#include "../header/my_queue.h"
#include "../header/threadHelper.h"

#define BLKSIZE 100
#define BLKSIZESERVANT 150
sig_atomic_t terminate = 0;
void signalHandler(int sig){
    terminate = 1;    
}
ServantInfo **servantInfos;
int numberOfServants = 0;
pthread_cond_t cond;
pthread_mutex_t mtx;
pthread_mutexattr_t mtxAttr;

pthread_mutex_t mtxServantInfos;
pthread_mutexattr_t mtxAttrServantInfos;
/*will hold fds of scckets trying to connect to the server*/
my_queue socketFds;
time_t curtime;
// ./server -p PORT -t numberOfThreads
int main(int argc , char* argv[]){
    
    if(argc != 5){
        perror("Invalid format, Please enter in this form: ./server -p PORT -t numberOfThreads\n");
        exit(1);
    }


    time(&curtime);
    servantInfos = (ServantInfo **) malloc(sizeof(ServantInfo *)* BLKSIZE );
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &signalHandler;
    if ((sigemptyset(&sa.sa_mask) == -1) ||  (sigaction(SIGINT,&sa, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    
    int sockfd , newsockfd, port_no ,numberOfThreads;
    
    struct sockaddr_in serv_addr , cli_addr;

    socklen_t clilen;

    port_no = atoi(argv[2]);
    numberOfThreads = atoi(argv[4]);
    if(my_queue_init(&socketFds) == -1){
        perror("Queue can not be initialized");
        return -1;
    }
    // initalize mutex and cond var
    if(initializeMutexAndCondVar()!=0){
        return -1;
    }

    // create threads
    pthread_t * threads = createThreads(numberOfThreads,serverThreadFunc,NULL); 
    if(threads == NULL){
        pthread_mutexattr_destroy(&mtxAttr);
        pthread_mutexattr_destroy(&mtxAttrServantInfos);


        pthread_mutex_destroy(&mtx);
        pthread_mutex_destroy(&mtxServantInfos);

        perror("threads could not be created server");
        /*for (int i = 0; i < numberOfRequests; i++)
        {
            free(requests[i]);
        }
        free(requests);
        free(threadArgs);*/
        return -1;
    }
    
    memset(&serv_addr,0,sizeof(serv_addr));
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if(sockfd < 0){
        perror("error opening socket\n");
        pthread_mutexattr_destroy(&mtxAttr);
        pthread_mutexattr_destroy(&mtxAttrServantInfos);


        pthread_mutex_destroy(&mtx);
        pthread_mutex_destroy(&mtxServantInfos);
        joinThreads(threads,numberOfThreads);
        free(threads);

        exit(1); 
        
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_no); 
    
    if(bind(sockfd , (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0 || terminate){
        if(!terminate)
            perror("error binding failed.\n");
        else{
            terminateMessage();
        }
        close(sockfd);
        pthread_mutexattr_destroy(&mtxAttr);
        pthread_mutexattr_destroy(&mtxAttrServantInfos);


        pthread_mutex_destroy(&mtx);
        pthread_mutex_destroy(&mtxServantInfos);
        joinThreads(threads,numberOfThreads);
        free(threads);
        exit(1);
    }
    

    if(listen(sockfd , 5) == -1 || terminate){
        if(!terminate)
            perror("listen");
        else
            terminateMessage();
        close(sockfd);
        pthread_mutexattr_destroy(&mtxAttr);
        pthread_mutexattr_destroy(&mtxAttrServantInfos);


        pthread_mutex_destroy(&mtx);
        pthread_mutex_destroy(&mtxServantInfos);
        joinThreads(threads,numberOfThreads);
        free(threads);
        return -1;
    }
    /*perror("c");
    socklen_t len = sizeof(serv_addr);
    //printf("PORTTTTT: %d %d ",getsockname(sockfd, (struct sockaddr *) &serv_addr,&serv_addr),errno);
    getsockname(sockfd, (struct sockaddr *) &serv_addr, &len);
    //inet_ntop(AF_INET, &serv_addr.sin_addr, myIP, sizeof(myIP));
    uint16_t myPort = ntohs(serv_addr.sin_port);

    //printf("Local ip address: %s\n", myIP);
    printf("Local port : %u\n", myPort);
    //printf("\nADDRD: %d\n",(struct sockaddr *)serv_addr.sin_port);
    perror("a");
    fflush(stdout);
    return 0;*/
    clilen = sizeof(cli_addr);

    // newsockfd = accept(sockfd , (struct sockaddr*) &cli_addr,&clilen);

    // if(newsockfd < 0){
    //     perror("error in accept\n");
    //     close(sockfd);
    //     exit(1);
    // }

    int counter = 0;
    while (1)
    {
        newsockfd = accept(sockfd , (struct sockaddr*) &cli_addr,&clilen);
        counter++;
        //("counter:: %d\n",counter);
        fflush(stdout);
        if(newsockfd < 0 || terminate){
            
            if(!terminate)
                perror("error in accept\n");
            else
                terminateMessage();

            //perror("ACCEPTTEN SONRAKI TERMINATE");
            killServants();
            for (int i = 0; i < numberOfServants ; i++)
            {
                //printf("SERVANT: %s %s %d %u\n",servantInfos[i]->firstCity,servantInfos[i]->lastCity,servantInfos[i]->pid,servantInfos[i]->portNum);
                free(servantInfos[i]);
            }
            free(servantInfos);
                
            
            close(sockfd);
            pthread_cond_broadcast(&cond);
            pthread_mutexattr_destroy(&mtxAttr);
            pthread_mutexattr_destroy(&mtxAttrServantInfos);


            pthread_mutex_destroy(&mtx);
            pthread_mutex_destroy(&mtxServantInfos);
            joinThreads(threads,numberOfThreads);
            free(threads);
            my_queue_destroy(&socketFds);

            exit(1);
        }
        // monitor bası
        // error check unutma
        if(pthread_mutex_lock(&mtx)!=0){

            perror("pthread_mutex_lock server main");
            close(newsockfd);
            close(sockfd);
            pthread_cond_broadcast(&cond);
            pthread_mutexattr_destroy(&mtxAttr);
            pthread_mutexattr_destroy(&mtxAttrServantInfos);


            pthread_mutex_destroy(&mtx);
            pthread_mutex_destroy(&mtxServantInfos);

            joinThreads(threads,numberOfThreads);
            free(threads);
            my_queue_destroy(&socketFds);
            killServants();

            return -1;
        }
        if(my_queue_insert(&socketFds,newsockfd) == -1){
            perror("error on reading\n");
            pthread_mutex_unlock(&mtx);

            pthread_mutexattr_destroy(&mtxAttr);
            pthread_mutexattr_destroy(&mtxAttrServantInfos);


            pthread_mutex_destroy(&mtx);
            pthread_mutex_destroy(&mtxServantInfos);
            close(newsockfd);
            close(sockfd);
            pthread_cond_broadcast(&cond);

            joinThreads(threads,numberOfThreads);
            free(threads);
            my_queue_destroy(&socketFds);
            killServants();
            return -1;
        }
        // send signal to one of the server threads
        if(pthread_cond_signal(&cond) !=0){
            pthread_mutex_unlock(&mtx);

            pthread_mutexattr_destroy(&mtxAttr);
            pthread_mutexattr_destroy(&mtxAttrServantInfos);
            pthread_mutex_destroy(&mtx);
            pthread_mutex_destroy(&mtxServantInfos);
            
            close(newsockfd);
            close(sockfd);
            pthread_cond_broadcast(&cond);
            joinThreads(threads,numberOfThreads);
            free(threads);
            my_queue_destroy(&socketFds);
            killServants();
            return -1;
        }

// ++arrived;
//     char lastOne = 1;
//     while(arrived < barrierForThreads ){
//         //printf("Thread %d,sonlandi bekliyor\n",arg->idOfThread);
//         pthread_cond_wait(&cond,&mtx);
//         lastOne = 0;
//     }
//     if(lastOne){
//         perror("BEN GELDIMMM");
//         //sleep(1);
//         pthread_cond_broadcast(&cond);
//     }


        if(pthread_mutex_unlock(&mtx)!=0){
            perror("pthread_mutex_unlock server main");
            close(sockfd);
            close(newsockfd);
            pthread_cond_broadcast(&cond);
            joinThreads(threads,numberOfThreads);
            free(threads);
            my_queue_destroy(&socketFds);

            pthread_mutexattr_destroy(&mtxAttr);
            pthread_mutexattr_destroy(&mtxAttrServantInfos);
            pthread_mutex_destroy(&mtx);
            pthread_mutex_destroy(&mtxServantInfos);
            killServants();
            return -1;
        }
        // monitor sonu
        
        // isteği aldım.
        // servant ın bilgisini de almış olabilir. bunun ayrımını thread yapcak.



        // queue ya koycam, monitorle sinyal yollıcak ,threadler halletcek
        // server'ın threadleri ilgili servant'a isteği yolladı. Servant cevabını soruyu gonderen server threadi beklıcek(read ile). cevabı alıp client'a yollıcak.


        // gene accept dicek 
        //zero(buffer , 255);
        /*memset(buffer,0,255);
        ClientThreadArg request;
        while(((n=read(newsockfd , &request,sizeof(ClientThreadArg))) == -1 ) && errno == EINTR);
        //n = read(newsockfd , buffer , 255);
        if(n < 0){
            perror("error on reading\n");
            close(newsockfd);
            close(sockfd);
            pthread_cond_broadcast(&cond);
            joinThreads(threads,numberOfThreads);
            free(threads);
            my_queue_destroy(&socketFds);

            return -1;
        }
        printf("Client: %s %s %d %d\n",(request.city[0] == '\0') ? "------":request.city ,request.type,request.start_date.day,n);
        */
        //sleep(1);
        /*
        if(n == 0){
            close(newsockfd);
            close(sockfd);
            //code repetition
            sockfd = socket(AF_INET , SOCK_STREAM , 0);

            if(sockfd < 0){
                perror("error opening socket\n");
                exit(1); 
            }

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = INADDR_ANY;
            serv_addr.sin_port = htons(port_no);
            
            if(bind(sockfd , (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
                perror("error binding failed.\n");
                close(sockfd);
                exit(1);
            }
            
            if(listen(sockfd , 5) == -1){
                perror("listen");
                close(sockfd);
                return -1;
            }
            clilen = sizeof(cli_addr);

            newsockfd = accept(sockfd , (struct sockaddr*) &cli_addr,&clilen);

            if(newsockfd < 0){
                perror("error in accept\n");
                close(sockfd);
                exit(1);
            }
        }
        */
        /*bzero(buffer , 255);
        fgets(buffer,255,stdin);

        n = write(newsockfd , buffer , strlen(buffer));
        
        if(n < 0){
            perror("error on writing\n");
        }
        */
        /*int i = strcmp("Bye" , buffer);
        for (int i = 0; i < strlen(buffer); i++)
        {
            printf("qq: %d %c\n",buffer[i],buffer[i]);
        }
        printf("\n");
        

        if(i == 0)
            break;   */

        //close(newsockfd);
    }
    pthread_cond_broadcast(&cond);

    joinThreads(threads,numberOfThreads);
    free(threads);
    close(newsockfd);
    close(sockfd);
    my_queue_destroy(&socketFds);
    pthread_mutexattr_destroy(&mtxAttr);
    pthread_mutexattr_destroy(&mtxAttrServantInfos);
    pthread_mutex_destroy(&mtx);
    pthread_mutex_destroy(&mtxServantInfos);

    return 0; 
}
//int servantSayisi =0;
int numberOfRequests =0;

void * serverThreadFunc(void *arg){
    //printf("SELAMUN ALEYKUM\n");
    
    while(1){
        pthread_mutex_lock(&mtx);
        while(my_queue_is_empty(&socketFds)){
            //printf("Thread %d,sonlandi bekliyor\n",arg->idOfThread);
            //perror("BBB");
            
            if(terminate){
                //terminateMessage();

                pthread_mutex_unlock(&mtx);
                return 0;
            }
            //printf("---servantSayisi, %d numberOfRequests %d\n",servantSayisi, numberOfRequests);

            pthread_cond_wait(&cond,&mtx);
            //perror("AAA");
            if(terminate){
                //terminateMessage();
                pthread_mutex_unlock(&mtx);
                return 0;
            }
        }
        
        //perror("Uyandı Elements");
        
        int newsockfd = 0;
        /*for(int i=socketFds.size-1;i>=0;i--){
            printf("%d, ",socketFds.arr[i]);
        }*/
        my_queue_remove(&socketFds,&newsockfd);
        /*printf("REM: %d\n",newsockfd);
        for(int i=socketFds.size-1;i>=0;i--){
            printf("%d, ",socketFds.arr[i]);
        }
        printf("\n");*/
        pthread_mutex_unlock(&mtx);
        //sleep(1);

        // if it is servant, record its data



        // read message
        char buffer[250];
        memset(buffer,0,250);
        //char control = 0;
        ClientThreadArg request;
        int n;
        //int ans = 0;
        uint8_t bit;
        while(((n=read(newsockfd , &buffer,250)) == -1 ) && errno == EINTR);
        //printf("n : %d sizeof(ClientThreadArg): %ld sizeof(ServantInfo): %ld\n",n,sizeof(ClientThreadArg),sizeof(ServantInfo));
        if(n == sizeof(ClientThreadArg)){
            //printf("BU bir client\n");
            bit = 0;
        } else if(n == sizeof(ServantInfo)){
            //printf("BU bir servant\n");
            bit = 1;
        } else{
            bit = -1;
            perror("Reading from socket error");
            //printf("QQQQQQQQQQQQQQQQBU NE BOYLE: %d\n",n);
        }


        //while(((n=read(newsockfd , &bit,sizeof(u_int8_t))) == -1 ) && errno == EINTR);
        //fprintf(stderr, "BIT READED: %d",newsockfd);
        //while(((n=read(newsockfd , &ans,sizeof(int))) == -1 ) && errno == EINTR);
        //n = read(newsockfd , buffer , 255);
        //printf("OKUNAN BYTE: %d\n",n);
        if(n < 0){
            perror("error on reading\n");
            close(newsockfd);
            //close(sockfd);
            //pthread_cond_broadcast(&cond);
            //joinThreads(threads,numberOfThreads);
            //free(threads);
            //my_queue_destroy(&socketFds);

            return NULL;
        }
        //memcpy(&bit,buffer,1);
        if(bit == 0){
            pthread_mutex_lock(&mtx);
            numberOfRequests++;
            pthread_mutex_unlock(&mtx);
            //while(((n=read(newsockfd , &request,sizeof(ClientThreadArg))) == -1 ) && errno == EINTR);
            //perror("");


            memcpy(&request,buffer,sizeof(ClientThreadArg));
            
            char format[] = "%s Request arrived 'transactionCount %s %d-%d-%d %d-%d-%d %s'\n";
            char buf[200];
            if(snprintf(buf,200,format,ctime(&curtime),request.type,request.start_date.day,request.start_date.month,request.start_date.year,request.end_date.day,request.end_date.month,request.end_date.year,request.city) < 0){
                perror("snprintf consumed");
                return NULL;
            }
            struct flock lock;
            memset(&lock,0,sizeof(lock));
            lock.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
            //printTime();
            while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
            //fprintf(stderr,"CLIENT OLDUGU ANLASILDI%d\n",newsockfd);
            //printf("SERVER THREAD: Client: %s %s %d %d\n",(request.city[0] == '\0') ? "------":request.city ,request.type,request.start_date.day,n);
            int answer = sendRequestToServant(&request);
            while(((n=write(newsockfd , &answer,sizeof(int))) == -1 ) && errno == EINTR);

        }
        else{
            /*pthread_mutex_lock(&mtx);
            //char buf[BLKSIZESERVANT];
            //memset(buf,0,BLKSIZESERVANT);
            servantSayisi++;
            //printf("servantSayisi, %d numberOfRequests %d\n",servantSayisi, numberOfRequests);

            pthread_mutex_unlock(&mtx);*/
            
            //while(((n=read(newsockfd , &ans,sizeof(int))) == -1 ) && errno == EINTR);
        
            //pthread_mutex_lock(&mtx);
            ServantInfo* serInfo = (ServantInfo*)malloc(sizeof(ServantInfo));
            //while(((n=read(newsockfd , serInfo,sizeof(ServantInfo))) == -1 ) && errno == EINTR);
            memcpy(serInfo,buffer,sizeof(ServantInfo));
            char format[] = "%s Servant %u present at port %d handling cities %s-%s\n";
            char buf[200];
            if(snprintf(buf,200,format,ctime(&curtime),serInfo->pid,serInfo->portNum,serInfo->firstCity,serInfo->lastCity) < 0){
                perror("snprintf error");
                return NULL;
            }
            struct flock lock;
            memset(&lock,0,sizeof(lock));
            lock.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
            //get_current_time();
            //while((write(STDOUT_FILENO,currenttime,strlen(currenttime)) == -1) && errno == EINTR);
            
            //printTime();
            while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);

            //fprintf(stderr,"SERVANT OLDUGU ANLASILDI%d\n",newsockfd);
            
            //printf("GELEN SERVANT:: -%s-%s- %u pid: %d %s",serInfo->firstCity,serInfo->lastCity,serInfo->portNum,serInfo->pid,serInfo->IP);
            
            //pthread_mutex_unlock(&mtx);
    
            // save servant info
            
            pthread_mutex_lock(&mtxServantInfos);
            servantInfos[numberOfServants] = serInfo;
            numberOfServants++;
            pthread_mutex_unlock(&mtxServantInfos);

        }
        


        // if it is client take its request and send it to the corresponding servant
    }
    //++arrived;
    
    // Threads is advancing to the second part
    //printf("Thread %d,devam ediyor\n
    return 0;

}
int initializeMutexAndCondVar(){
    int s;
    s = pthread_mutexattr_init(&mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_init");
        return -1;
    }
    s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_ERRORCHECK);
    if (s != 0){
        perror("pthread_mutexattr_settype");

        return -1;
    }
    s = pthread_mutex_init(&mtx, &mtxAttr);
    if (s != 0){
        perror("pthread_mutex_init");

        return -1;
    }
    //---
    s = pthread_mutexattr_init(&mtxAttrServantInfos);
    if (s != 0){
        perror("pthread_mutexattr_init");
        return -1;
    }
    s = pthread_mutexattr_settype(&mtxAttrServantInfos, PTHREAD_MUTEX_ERRORCHECK);
    if (s != 0){
        perror("pthread_mutexattr_settype");

        return -1;
    }
    s = pthread_mutex_init(&mtxServantInfos, &mtxAttrServantInfos);
    if (s != 0){
        perror("pthread_mutex_init");

        return -1;
    }
    //--
    if(pthread_cond_init(&cond, NULL) != 0){
        s = pthread_mutexattr_destroy(&mtxAttr); /* No longer needed */
        if (s != 0)
            perror("pthread_mutexattr_destroy");
        s = pthread_mutexattr_destroy(&mtxAttrServantInfos); /* No longer needed */
        if (s != 0)
            perror("pthread_mutexattr_destroy");
        
        return -1;
    }
    return 0;
}
void killServants(){
    for (int i = 0; i < numberOfServants ; i++)
    {
        kill(servantInfos[i]->pid,SIGINT);
    }
}
int sendRequestToServant(ClientThreadArg* request){
    if(request->city[0] == '\0'){
        // send request to all servants
        int sum = 0;
        for(int i=0;i<numberOfServants;i++){
            sum += sendRequestToServantHelper(servantInfos[i]->portNum,servantInfos[i]->IP,request,sizeof(ClientThreadArg));
        }
        char format[]="%sContacting ALL servants\n";
        char buf[100];
        if(snprintf(buf,100,format,ctime(&curtime)) < 0){
            perror("snprintf consumed");
            return -1;
        }
        //char buf[]="%sContacting ALL servants\n";
        struct flock lock;
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
        lock.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        //printf("571 * 9 = %d",sum);
        return sum;
    } else{
        // send the corresponding servant
        int index = indexOfServant(request->city);
        if(index == -1){
            perror("servant does not exist\n");
            return -1;
        }
        char buf[100];
        char format[]="%sContacting servant %d\n";
        if(snprintf(buf,100,format,ctime(&curtime),servantInfos[index]->pid) < 0){
            perror("snprintf consumed");
            return -1;
        }

        struct flock lock;
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);
        while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
        lock.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lock);

        //printf("CITY : %s -> %s %s: \n",request->city,servantInfos[index]->firstCity,servantInfos[index]->lastCity);
        return sendRequestToServantHelper(servantInfos[index]->portNum,servantInfos[index]->IP,request,sizeof(ClientThreadArg));
    }
    return -1;
}
int indexOfServant (char * cityName){
    for (int i = 0; i < numberOfServants ; i++)
    {
        if(strcmp(servantInfos[i]->firstCity,cityName)<= 0 && strcmp(servantInfos[i]->lastCity,cityName)>=0){
            return i;
        }
    }
    
    return -1;
}
int sendRequestToServantHelper(int portNumber,char const * IP,void * request,int size){

    struct sockaddr_in serv_addr;
    //char buffer[BUF_SIZE];
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket");
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
        terminateMessage();
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
    //uint8_t bit = 0;
    int n;
    /*struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;*/
    //while((n=write(sockfd , &bit,sizeof(u_int8_t)) == -1 ) && errno == EINTR);
    while((n=write(sockfd , request,size) == -1 ) && errno == EINTR);
    int answer = 0;
    while((n=read(sockfd , &answer,sizeof(int)) == -1 ) && errno == EINTR);
    char format[] = "%sResponse received: %d, forwarded to client\n";
    char buf[100];
    if(snprintf(buf,100,format,ctime(&curtime),answer) < 0){
        perror("snprintf consumed");
        return -1;
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    //printf("ANSWER OF SERVANT IN SERVER THREAD: %d\n",answer);
    /*lock.l_type = F_UNLCK;
    fcntl(sockfd,F_SETLKW,&lock);*/
    
    close(sockfd);
    return answer;
}
void printTime(){


    //printf("Current time = %s ", ctime(&curtime));
    char * time = ctime(&curtime);
    while((write(STDOUT_FILENO,time,strlen(time)-1) == -1) && errno == EINTR);
    while((write(STDOUT_FILENO," ",sizeof(char)) == -1) && errno == EINTR);

    
}
/*void get_current_time(){

    int ms;
    char str_time[100];
    struct timeval cur_t1;

    gettimeofday(&cur_t1, NULL);
    ms = ( cur_t1.tv_usec / 1000 );

    strftime(str_time, 100, "%H:%M:%S", localtime(&cur_t1.tv_sec));
    sprintf(currenttime, "%s:%03d", str_time, ms);
}*/
            
void terminateMessage(){
    char format[] = "%sSIGINT has been received. I handled a total of %d requests. Goodbye.\n";
    char buf[200];
    if(snprintf(buf,200,format,ctime(&curtime),numberOfRequests) < 0){
        perror("snprintf consumed");
        return;
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    //printTime();
    while((write(STDOUT_FILENO,buf,strlen(buf)) == -1) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
}