#include "../header/servant.h"
#include "../header/client.h"
#include "../header/Container.h"
#include "../header/threadHelper.h"
#include "../header/Date.h"
#include "../header/hashmap.h"



#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include <arpa/inet.h>
#include<signal.h>

#define BUFSIZE 100
#define BLKSIZE150 150

// ./servant -d datasetPath -c 10-19 -r IP -p port
// IP and portnum to make server know how and when to reach servant 
char const * IP; // argv [6]
int portNum; // argv[8]
uint16_t uniquePortNum;

pthread_t ** threads;
int threadsCapacity = 2;
int threadsSize = 0;
sig_atomic_t terminate = 0;
void signalHandler(int sig){
    terminate = 1;    
}
Container * cities = NULL;
int numberOfCities;
int sizeOfHashMap=0;

struct DataItem** hashArray;
void saveToHashmap(struct DataItem ** hashArray,Container * cities);
int main(int argc, char const *argv[])
{
    
    /*for (int i = 0; i < 20; i++)
    {
        int a =creattUniquePort();
    }
    return 0;*/
    //perror("SERVANT CALISTI");
    if(argc != 9){
        perror("Invalid arguments\nExample: ./servant -d directoryPath -c 10-19 -r IP -p PORT\n");
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
    portNum = atoi(argv[8]);
    IP = argv[6];
    threads = (pthread_t**) malloc(sizeof(pthread_t*)*threadsCapacity);
    if(threads == NULL){
        perror("malloc error");
        return -1;
    }


    //struct dirent **namelist;
    char const * dataSetPath = argv[2];
    
    int startEndIndexes[2];
    int len = strlen(argv[4]);
    // take indexes from command line arguments
    char *copyStr = (char*) malloc(len * sizeof(char));
    if(copyStr == NULL){
        perror("Malloc error copyStr\n");
        return -1;
    }
    strcpy(copyStr,argv[4]);
    if(takeStartEndIndexes(copyStr,startEndIndexes) == -1){
        perror("Indexes are not correct\n");
        free(copyStr);
        return -1;
    }
    numberOfCities = startEndIndexes[1] - startEndIndexes[0] +1;

    //printf("%d %d \n",startEndIndexes[0],startEndIndexes[1]);
    free(copyStr);

    // take dataset file names
    int i,n;
    DIR *dir;
    int numberOfDirectories = 0;
    //struct dirent entry[100];
    
    struct dirent * temp;
    int directoriesLength = getNumberOfDirectories(dataSetPath);
    if(directoriesLength == -1){
        free(threads);
        return -1;
    }
    struct dirent *entry = (struct dirent *)malloc(sizeof(struct dirent ) * directoriesLength);
    if(entry == NULL){
        perror("malloc error");
        free(threads);
        return -1;
    }
    hashArray = (struct DataItem **)calloc(sizeof(struct DataItem *),directoriesLength);
    if(hashArray == NULL){
        perror("malloc error");
        free(threads);
        free(entry);
    }
    sizeOfHashMap = directoriesLength;
    if ((dir = opendir(dataSetPath)) == NULL){
        perror("opendir() error");
        closedir(dir);
        free(threads);
        return -1;
    }
    else {
        //puts("contents of root:");
        int i=0;
        while ((temp= readdir(dir)) != NULL){
            //fprintf(stderr,"i: %d %s\n",i,temp->d_name);
            //entry[i++] = temp;
            //printf("%d,%s\n",i,temp->d_name);
            //if(i>1){
            strcpy(entry[i].d_name,temp->d_name);
            //printf("%d ,%s",i,entry[i].d_name);
            numberOfDirectories++;

                //entry[i-2] = *temp;
                
                //printf("ENTRY: %s \n",entry[i]);
            //}
            i++;
        }
        /*for (int j = 0; j < numberOfDirectories; j++)
        {
            printf("Q: %s\n",entry[j].d_name);
        }*/
        
        qsort(entry,numberOfDirectories,sizeof(struct dirent),direntCompare);
        /*for(int x = 0;x < numberOfDirectories;x++){
            printf("AA: %d:   %s\n",x, entry[x].d_name);
            i++;
        }*/
        closedir(dir);
    }
    // for (int i = 0; i < numberOfDirectories; i++)
    // {
    //     printf("%d -> %s\n",i,entry[i].d_name);
    // }
    
    // if(numberOfDirectories != 81){
    //     perror("Dataset changed\n");
    //     return -1;
    // }
    //fprintf(stderr,"+");
    
    //fflush(stdout);
    //return 0;
    //n = scandir(dataSetPath, &namelist, 0, alphasort);
    n = numberOfDirectories;
    
    if (n < 0){
        perror("Data could not be read");
        //free(namelist); // in document they had free it. 
        free(entry);
        freeHashArray(hashArray,sizeOfHashMap);
        free(threads);
        return -1;
    }
    else if(startEndIndexes[0] < 1 ||  startEndIndexes[0] >= n || startEndIndexes[1]<1  || startEndIndexes[1] >= n-1 || startEndIndexes[0] > startEndIndexes[1]){
        perror("Start or/and end index is not valid\n");
        // for (i = 0; i < n; i++) {
        //     free(namelist[i]);
        // }
        // free(namelist);
        free(entry);
        freeHashArray(hashArray,sizeOfHashMap);

        free(threads);


        return -1;
    }
    else {
        cities = (Container *) malloc(numberOfCities * sizeof(Container));
        if(cities == NULL){
            perror("Malloc error");
            // for (i = 0; i < n; i++) {
            //     free(namelist[i]);
            // }
            // free(namelist);
            return -1;
        }
        /*for (int i = 0; i < numberOfDirectories; i++)
        {
            printf("%d -> %s\n",i,entry[i].d_name);
        }*/
        /*printf("start end indexes: %d %d\n",startEndIndexes[0],startEndIndexes[1]);
        fprintf(stderr,"start: %s end: %s",entry[startEndIndexes[0]+1].d_name,entry[startEndIndexes[1]+1].d_name);
        fprintf(stderr,"!!!:\n");*/
        for (i = 2; i < n; i++) {
            if(i > startEndIndexes[0] && i <= startEndIndexes[1]+1){
                //printf("%d %s\n", x++,namelist[i]->d_name);
                //printf("ARALIK::%d %s\n",i,entry[i].d_name);
                //perror("q");
                //printf("DATASETPATH: %s\n",dataSetPath);
                if(saveCity(dataSetPath,cities,i-startEndIndexes[0]-1,entry[i].d_name) == -1){
                    perror("Error in SaveCity");
                    // for (i = 0; i < n; i++) {
                    //     free(namelist[i]);
                    // }
                    // free(namelist);
                    free(cities);
                    free(entry);
                    
                    freeHashArray(hashArray,sizeOfHashMap);

                    free(threads);
                    return -1;
                }
                //perror("1q");
                


            }
        }
        //fprintf(stderr,"___:n");
    }

    saveToHashmap(hashArray,cities);
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    fprintf(stdout,"Servant %d: loaded dataset, cities %s-%s\n",getProcessId(),cities[0].nameOfCity,cities[numberOfCities-1].nameOfCity);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);

    free(entry);

    /*char format[] = "Servant: %d: Loaded dataset, cities %s %s'\n";
    char buf[200];
    if(snprintf(buf,200,format,getProcessId(),cities[0].nameOfCity,cities[numberOfCities-1].nameOfCity) < 0){
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
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);*/

    char buffer[BLKSIZE150];
    memset(buffer,'\0',BLKSIZE150);
    strcpy(buffer,cities[0].nameOfCity);
    strcat(buffer," ");
    strcat(buffer,cities[numberOfCities-1].nameOfCity);
    //fprintf(stderr,"SERVANT SEND DEN HEMEN ONCE");
    int x = sendConnectionInfoToServer(buffer,BLKSIZE150);
    if(x != -1) 
        waitForRequests();
    
    //int fd createSocket();
    //printf("ID \n"
    //cities[0].transactionFiles[0].transactions[0].id);
    //;
    //fflush(stdout);

    // for (i = 0; i < n; i++) {
    //     free(namelist[i]);
    // }
    // free(namelist);
    freeCities(cities,numberOfCities);
    free(threads);
    freeHashArray(hashArray,sizeOfHashMap);
    //joinThreads(threads,threadsSize);

    // for (int i = 0; i < numberOfCities; i++)
    // {
    //     for(int j=0;j<cities[i].numberOfTransactionFiles;j++){
    //         free(cities[i].transactionFiles[j].transactions);
    //     }
    //     free(cities[i].transactionFiles);

    // }

    return 0;
}
int takeStartEndIndexes(char * str,int * startEndIndexes){
    if(str == NULL)
        return -1;

    char const * separator = "-";
    char *token;
    /* get the first token */
    token = strtok(str, separator);
   
    /* walk through other tokens */
    for (int i = 0; i < 2 ; i++)
    {
        if(token == NULL || !isNumber(token))
            return -1;
        startEndIndexes[i] = atoi(token);

        token = strtok(NULL, separator);
    }
    //printf("aa %d %d\n",startEndIndexes[0],startEndIndexes[1]);
    return 0;
}
int isNumber(char const *s)
{
    for (int i = 0; s[i]!= '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
              return 0;
    }
    return 1;
}
int saveCity(char const * dataSetDirName,Container * cities,int index,char* cityName){
    //printf("%d index %s\n",index,cityName);
    //perror("!!!!!!111111111");
    
    strcpy(cities[index].nameOfCity,cityName);
    //printf("sehir:%s\n",cities[index].nameOfCity);
    //perror("!!!!!!1");
    char transactionDateFileName[BUFSIZE];
    memset(transactionDateFileName,'\0',BUFSIZE);
    /*if(transactionDateFileName == NULL){
        perror("Malloc error");
        return -1;
    } */
    // initialize fileName
    //perror("q");

    //printf("cityName: %s %s \n",cityName,dataSetDirName);
    //perror("qwq");
    
    strcpy(transactionDateFileName,dataSetDirName);
    //("c");
    
    strcat(transactionDateFileName,"/");
    
    //perror("b");
    
    strcat(transactionDateFileName,cityName);
    //perror("a");
    //printf("%s",transactionDateFileName);
    /*strcpy(transactionDateFileName+lenDataSetDir,"/");
    strcpy(transactionDateFileName+lenDataSetDir+1,cityName);*/
    //printf("%s + %s transactionDateFileName: %s\n",dataSetDirName,cityName,transactionDateFileName);
    //perror("t");

    int n;
    //struct dirent files[10];
    struct dirent* temp;
    //perror("qqqqq");
    int numberOfFilesInDir = numberOfFilesInDirectory(transactionDateFileName);
    //printf("numberOfFiles:%d sizeof(TransactionFile) * numberOfFilesInDir=%d\n",numberOfFilesInDir,sizeof(TransactionFile) * numberOfFilesInDir);
    //perror("aaaa");
    
    cities[index].transactionFiles = (TransactionFile *)malloc(sizeof(TransactionFile) * numberOfFilesInDir);
    if(cities[index].transactionFiles == NULL){
        perror("malloc error");
        return -1;
    }
    /*for (int i = 0; i < numberOfFilesInDir; i++)
    {
        perror("cities");
        fprintf(stderr,"i: %d size:%d offset: %d\n",i,sizeof(TransactionFile),sizeof(TransactionFile) * i);
        perror("QQQ");
        //memset(&(cities[index].transactionFiles[i]),'\0',sizeof(TransactionFile));
        perror("11111");
    
    }*/
    
    cities[index].numberOfTransactionFiles = numberOfFilesInDir;
    /*if(cities[index].transactionFiles == NULL){
        perror("malloc error cities[index].transactionFiles");
        return -1;
    }*/
    //perror("")
    //printf("numberOfFilesInDir: %d\n", numberOfFilesInDir);
    DIR *dir;
    //perror("BBBBB");
    int numberOfFiles = 0;
    //perror("(((");
    //fprintf(stderr,"transactionDateFileName:%s\n",transactionDateFileName);
    if ((dir = opendir(transactionDateFileName)) == NULL){
        perror("opendir() error");
        closedir(dir);
        free(cities[index].transactionFiles);

        return -1;
    }
    else {
        //puts("contents of root:");
        //perror("INN");

        int i=0;
        while ((temp= readdir(dir)) != NULL){
            //fprintf(stderr,"i: %d %s\n",i,temp->d_name);
            //entry[i++] = temp;
            
            /*if(i>1){
                strcpy(files[i-2].d_name,temp->d_name);
                numberOfFiles++;

                //entry[i-2] = *temp;
                
                //printf("ENTRY: %s \n",entry[i]);
            }*/
            if(strcmp(temp->d_name,".") != 0 && strcmp(temp->d_name,"..") != 0){
                //fprintf(stderr,"temp->d_name: %s\n",temp->d_name);
                
                //memcpy(cities[index].transactionFiles[i].date,'\0',20);
                strcpy(cities[index].transactionFiles[i].date,temp->d_name);

                //perror("qqqqq");
                i++;
                numberOfFiles++;
            }
        }
        /*for (int j = 0; j < 10; j++)
        {
            printf("Q: %s\n",files[j].d_name);
        }*/
        
        //qsort(files,81,sizeof(struct dirent),direntCompare);
        closedir(dir);
    }
    //perror(")))");

    //perror("CCCCC");

   // n = scandir(transactionDateFileName, &namelist, 0, alphasort);
    //return 0;
    n = numberOfFiles;
    if (n < 0){
        perror("scandir");
        free(cities[index].transactionFiles);
        //free(namelist); // in document they had free it. 
        return -1;
    } else{
        // start from 2 because, first to files are . and ..
        // save the file
        for (int i = 0; i < n; i++)
        {
            char fileNamePath[BUFSIZE];
            strcpy(fileNamePath,transactionDateFileName);
            strcat(fileNamePath,"/");

            strcat(fileNamePath,cities[index].transactionFiles[i].date);
            //printf("cities[index].transactionFiles[i].date: %s\n",cities[index].transactionFiles[i].date);
            //printf("file %d : %s\n",i-1,fileNamePath);

            //strcpy(cities[index].transactionFiles[i].date,files[i].d_name);
            //perror("SAVE DATAFİLE");
            //perror("DDDD");

            if(saveDateFile(&cities[index].transactionFiles[i],fileNamePath) == -1){
                perror("saveDateFile");
                return -1;
            }
            //perror("QQQQ");



        }
        //printf("\n");
    }
    
    
    /*else if(startEndIndexes[0] < 1 ||  startEndIndexes[0] >= n || startEndIndexes[1]<1  || startEndIndexes[1] >= n-1 || startEndIndexes[0] > startEndIndexes[1]){
        perror("Start or/and end index is not valid\n");
        for (i = 0; i < n; i++) {
            free(namelist[i]);
        }
        free(namelist);
        return -1;
    }*/


    //free(cities[index].transactionFiles);

    //free(transactionDateFileName);
    return 0;


}
/*
    saves the given file to given transactionFile object
*/
int saveDateFile(TransactionFile * transactionFile,char * file_path){
    struct flock lock;
    int numberOfTransactions = getNumberOfTransactionsFromFile(file_path);
    if(numberOfTransactions == -1){
        perror("getNumberOfTransactionsFromFile");
        return -1;
    }
    //printf("numberOFTRANsactions;: %d size: %d total: %d\n",numberOfTransactions,sizeof(Transaction),sizeof(Transaction) * numberOfTransactions);
    transactionFile->transactions = (Transaction *) malloc(sizeof(Transaction) * numberOfTransactions);
    if(transactionFile->transactions == NULL){
        perror("malloc error");
        return -1;
    }
    transactionFile->numberOfTransactions = numberOfTransactions;
    //strcat(path,)
            
    //perror("FİLEPATH:");
    //printf("filepath: %s\n",file_path);

    int fd = open(file_path,O_RDONLY);
    //perror("FİLEPATHden sonra:");

    if(fd == -1){
        perror("open file error");
        return -1;
    }
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    /*Place a read lock on the file. */

    fcntl(fd,F_SETLKW,&lock);
    char buf[BLKSIZE150];
    int bytesread=0;
    int index = 0;
    for(;;){
        /*if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }*/
        memset(buf,'\0',BLKSIZE150);
        while(((bytesread = read(fd,buf,BLKSIZE150)) == -1) && (errno == EINTR)); // try to read until become successful
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
        //printf("LINE::: %s %d %d\n",buf,index,transactionFile->transactions[index].id);
        takeTransactionInfoFromLine(buf,&transactionFile->transactions[index]);
        index++;
        //printf("::%s,\n",transactionFile->transactions[index].nameOfStreet);
    }
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    if(close(fd) == -1){
        perror("close file error");
        return 1;
    }
    return 0;
}
    
int takeTransactionInfoFromLine(char * line,Transaction *tr){
    char *token;
    /* get the first token */
    char const * separator = " ";
    token = strtok(line, separator);

    /* walk through other tokens */
    for (int i = 0; i<5 && token!= NULL; i++)
    {
        if(token == NULL)
            return -1;
        //startEndIndexes[i] = atoi(token);
        //printf("%s-------\n",token);
        if(saveToTransaction(tr,token,i) == -1){
            return -1;
        }

        token = strtok(NULL, separator);
    }
    //printTransaction(tr);   
    return 0;
//    printf("STRUCT::%d %s %s %d %d\n",tr->id, tr->type,tr->nameOfStreet,tr->surfaceSquare,tr->price);

}
/*
struct Transaction{
    int id;         // 0
    char * type;    // 1
    char * nameOfStreet; // 2
    int surfaceSquare; // 3
    int price; // 4
};
*/
int saveToTransaction(Transaction * tr,char * str,int offset){
    switch (offset)
    {
    case 0:
        tr->id = atoi(str);
        break;
    case 1:
        strcpy(tr->type,str);
        break;
    case 2:
        strcpy(tr->nameOfStreet,str);
        break;
    case 3:
        tr->surfaceSquare = atoi(str);
        break;
    case 4:
        tr->price = atoi(str);
        break;
    default:
        perror("invalid offset saveToTransaction");
        return -1;
    }   
    return 0;
}
static int direntCompare(const void *p1, const void *p2)
{
    struct dirent* dirent_a = ( (struct dirent*) p1 );
    struct dirent* dirent_b = ( (struct dirent*) p2 );
    //return 1;
    //printf(" %s %s = %d,",dirent_a->d_name,dirent_b->d_name,strcmp(dirent_a->d_name,dirent_b->d_name));
    return strcmp(dirent_a->d_name,dirent_b->d_name);
}
int numberOfFilesInDirectory(char *fileName){
    DIR *dir;
    struct dirent* temp;

    int numberOfFiles = 0;
    //perror("222");
    //fprintf(stderr,"filename: %s\n",fileName);
    /*for (int i = 0; i < strlen(fileName); i++)
    {
        printf("%c->%d\n",fileName[i],fileName[i]);
    }*/
    
    if ((dir = opendir(fileName)) == NULL){
        perror("opendir() error");
        closedir(dir);
        return -1;
    }
    else {
        //puts("contents of root:");
        //perror("3333222");

        while ((temp= readdir(dir)) != NULL){
            if(strcmp(temp->d_name,".") != 0 && strcmp(temp->d_name,"..") != 0){
                numberOfFiles++;
            }
        }
        //perror("yyyyyyy");

        if(closedir(dir)==-1){
            perror("closedir");
            return -1;
        }
        
    }
    //perror("RETURN");
    return numberOfFiles;
}
int getNumberOfTransactionsFromFile(char const * fileName){
    int fd = open(fileName,O_RDONLY);
    if(fd == -1){
        perror("open file error");
        return -1;
    }
    struct flock lock;
    int numberOfTransactions = 0;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    /*Place a read lock on the file. */

    fcntl(fd,F_SETLKW,&lock);
    char buf[BLKSIZE150];
    int bytesread=0;
    for(;;){
        /*if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }*/
        memset(buf,'\0',BLKSIZE150);
        while(((bytesread = read(fd,buf,BLKSIZE150)) == -1) && (errno == EINTR)); // try to read until become successful
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
        //printf("LINE::: %s\n",buf);
        //takeTransactionInfoFromLine(buf,&transactionFile->transactions[index]);
        //printf("::%s,\n",transactionFile->transactions[index].nameOfStreet);
        numberOfTransactions++;
    }
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    if(close(fd) == -1){
        perror("close file error");
        return -1;
    }
    return numberOfTransactions;
}
void freeCities(Container * cities, int size){
    for (int i = 0; i < size; i++)
    {
        for(int j=0;j<cities[i].numberOfTransactionFiles;j++){
            free(cities[i].transactionFiles[j].transactions);
        }
        free(cities[i].transactionFiles);

    }
    free(cities);
}

int sendConnectionInfoToServer(char * message,int size){
    //printf("IP:: %s port : %d\n",IP,portNum);
    /*int sockfd , newsockfd , n;
    char buffer[255];
    
    struct sockaddr_in serv_addr;
    struct hostent *server;
    //perror("2");

    //port_no = port_no;

    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    //perror("3");

    if(sockfd < 0){
        perror("error opening socket\n");
        return -1; 
    }
    
    server = gethostbyname(IP);


    if(server == NULL){
        perror("error no such host\n");
        return -1;
    }

    //perror("1");

    //bzero((char*) &serv_addr , sizeof(serv_addr));
    memset((char*) &serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    memcpy((char*) server->h_addr,(char *) &serv_addr.sin_addr.s_addr, server->h_length);
    bcopy((char*) server->h_addr , (char *) &serv_addr.sin_addr.s_addr , server->h_length);
    serv_addr.sin_port = htons(portNum);*/
    //perror("Connectten once");
    //perror("122");
    struct sockaddr_in serv_addr;
    //char buffer[BUF_SIZE];
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket");
        return -1;
    }


    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNum);

    inet_pton(AF_INET, IP, &serv_addr.sin_addr);

    while(connect(sockfd , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) == -1){
        close(sockfd);
        /*if(terminate)
            break;*/
        sockfd = socket(AF_INET , SOCK_STREAM , 0);
    }
    //perror("12222");

    //perror("Connectten sonra");
    /*if(terminate){
        perror("TERMINATEEEE");

        close(sockfd);
    
        return 0;
    }*/
    //while (1)
    /*for(int i=0;i<numberOfRequests;i++)
    {
        //memset(buffer,0,255);
        //fgets(buffer,255,stdin);
        while((n=write(sockfd , requests[i],BLKSIZE150) == -1 ) && errno == EINTR);
        printf("YAZILDI: %s\n",requests[i]);

        if(n < 0){
            perror("error on writing");
            return -1;
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
    uniquePortNum = createUniquePortNumber();
    int pid = getProcessId();

    
    ServantInfo si;
    /*for (int i = 0; i < MAX_CITY_NAME_LENGTH; i++)
    {
        if(message[i] != ' '){
            si.firstCity[i] = message[i];
            message[i]='\0';
            indexOfSecondCity = i+1;
        }
    }*/
    char * first = strtok(message," ");
    strcpy(si.firstCity,first);    
    char * last = strtok(NULL," ");
    strcpy(si.lastCity,last);    
    
    si.pid = pid;
    si.portNum = uniquePortNum;
    
    strcpy(si.IP,IP);
    
    //si.bit = bit;
    /*struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;

    fcntl(sockfd,F_SETLKW,&lock);*/
    //while((n=write(sockfd , &bit,sizeof(uint8_t)) == -1 ) && errno == EINTR);

    while((n=write(sockfd , &si,sizeof(ServantInfo)) == -1 ) && errno == EINTR);
    /*lock.l_type = F_UNLCK;
    fcntl(sockfd,F_SETLKW,&lock);*/
    //printf("PİD ::: %d\n",pid);
    /*struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;

    fcntl(sockfd,F_SETLKW,&lock);
    
    while((n=write(sockfd , &bit,sizeof(u_int8_t)) == -1 ) && errno == EINTR);
    //strcpy(buf)
    //while((n=write(sockfd , &x,sizeof(int)) == -1 ) && errno == EINTR);
    while((n=write(sockfd , message,size) == -1 ) && errno == EINTR);
    
    while((n=write(sockfd , &uniquePortNum,sizeof(uint16_t)) == -1 ) && errno == EINTR);
    
    while((n=write(sockfd , &pid,sizeof(int)) == -1 ) && errno == EINTR);
    lock.l_type = F_UNLCK;
    fcntl(sockfd,F_SETLKW,&lock);*/
    //perror("qwe");
    
    close(sockfd);
    return 0;
}
uint16_t createUniquePortNumber(){
    struct sockaddr_in serv_addr;
    int sockfd;

    memset(&serv_addr,0,sizeof(serv_addr));
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if(sockfd < 0){
        perror("error opening socket\n");
        return -1; 
        
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;
    
    if(bind(sockfd , (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        perror("error binding failed. createUniquePortNumber");
        close(sockfd);
        return -1;
    }
    

    if(listen(sockfd , 5) == -1){
            perror("listen");
        close(sockfd);
        return -1;
    }
    socklen_t len = sizeof(serv_addr);
    //printf("PORTTTTT: %d %d ",getsockname(sockfd, (struct sockaddr *) &serv_addr,&serv_addr),errno);
    //inet_ntop(AF_INET, &serv_addr.sin_addr, myIP, sizeof(myIP));
    
    getsockname(sockfd, (struct sockaddr *) &serv_addr, &len);
    uint16_t myPort = ntohs(serv_addr.sin_port);

    //printf("Local ip address: %s\n", myIP);
    //printf("Local port : %u\n", myPort);
    //printf("\nADDRD: %d\n",(struct sockaddr *)serv_addr.sin_port);
    fflush(stdout);
    close(sockfd);
    return myPort;
}

    
int waitForRequests(){
    int sockfd , newsockfd;
    
    struct sockaddr_in serv_addr , cli_addr;

    socklen_t clilen;

    // create threads
    /*pthread_t * threads = createThreads(numberOfThreads,serverThreadFunc,NULL); 
    if(threads == NULL){
        perror("threads could not be created server");
        return -1;
    }*/
    
    memset(&serv_addr,0,sizeof(serv_addr));
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if(sockfd < 0){
        perror("error opening socket\n");
        /*joinThreads(threads,numberOfThreads);
        free(threads);*/
        return -1; 
        
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(uniquePortNum); 
    
    if(bind(sockfd , (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0 || terminate){
        if(!terminate)
            perror("error binding failed. wait for requests");
        close(sockfd);
        /*joinThreads(threads,numberOfThreads);
        free(threads);*/
        return -1;
    }
    

    if(listen(sockfd , 5) == -1 || terminate){
        if(!terminate)
            perror("listen");
        close(sockfd);
        /*joinThreads(threads,numberOfThreads);
        free(threads);*/
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
    //     return -1;
    // }
    int counter = 0;
    if(terminate){
        //perror("Servant sonlaniyor");
        close(sockfd);
        return -1;
    }
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    fprintf(stdout,"Servant %d: listening at port %u\n",getProcessId(),uniquePortNum);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lock);
    while (1)
    {
        //fprintf(stderr,"SERVANT BEKLIYORRRRRRRRRRRRRr\n");

        newsockfd = accept(sockfd , (struct sockaddr*) &cli_addr,&clilen);

        fflush(stdout);
        if(newsockfd < 0 || terminate){
            if(!terminate)
                perror("error in accept\n");
            for (int i = 0; i < threadsSize; i++)
            {
                //perror("FREE YAPIYOR");
                if(pthread_join(*threads[i],NULL) != 0){
                    perror("pthread_join");
                    // do not end the program
                }
                free(threads[i]);
            }
            freeHashArray(hashArray,sizeOfHashMap);

            hashArray = NULL;
            struct flock lock;
            memset(&lock,0,sizeof(lock));
            lock.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
            fprintf(stdout,"Servant %d: termination message received, handled %d requests in total.\n",getProcessId(),counter);
            lock.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lock);
            //free(threads);
            close(sockfd);
            //freeCities(cities,numberOfCities);
            return -1;
        }
        //ClientThreadArg request;
        createThread(newsockfd);
        counter++;
        //close(newsockfd);
    }
} 
int getProcessId(){
    int fd = open("/proc/self/stat",O_RDONLY);
    if(fd == -1){
        perror("open");
        return -1;
    }
    int maxlength = 50;
    char pidStr[maxlength];
    memset(pidStr,'\0',maxlength);
    while((read(fd,pidStr,maxlength-1) == -1) && errno == EINTR);
    //printf("strpid : %s\n",pidStr);
    for (int i = 0; i < maxlength; i++)
    {
        if(pidStr[i] == ' '){
            pidStr[i] = '\0';
            break;
        }
    
    }
    int pid = atoi(pidStr);
    close(fd);
    return pid;
}
        
int createThread(int sockfd){
    pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));
    if(thread == NULL){
        perror("malloc error");
        return -1;
    }
    if(threadsSize == threadsCapacity){
        //perror("thread yerleri doldu, genisletilmeli");
        int newCap = threadsCapacity *2;
        pthread_t ** temp = (pthread_t**) malloc(sizeof(pthread_t*)*newCap);
        if(temp == NULL){
            free(thread);
            return -1;
        }
        for (int i = 0; i < threadsCapacity; i++)
        {
            temp[i] = threads[i];
        }
        free(threads);
        threads = temp;
        threadsCapacity = newCap;
    }
    threads[threadsSize] = thread;
    threadsSize++;

    int * sockfd_p = (int*)malloc(sizeof(int));
    if(sockfd_p == NULL){
        perror("malloc error");
        free(thread);
        return -1;
    }
    *sockfd_p = sockfd;


    if(pthread_create(thread,NULL,servantThread,(void*)sockfd_p) != 0){
        perror("pthread_create");
        // wait for threads created so far
        /*for (int j = 0; j < i; j++)
        {
            pthread_join(threads[j],NULL);
        }
        free(threads);*/
        free(thread);
        free(sockfd_p);
        return -1;
    }
    
    return 0;
}
void * servantThread(void *arg){
    printf("%d",*(int*)arg);
    int n;
    int newsockfd = *(int*)arg;
    ClientThreadArg request;
    while(((n=read(newsockfd , &request,sizeof(ClientThreadArg))) == -1 ) && errno == EINTR);
    //printf("SERVANT thread ALDI SERVERDAN: %s %s %d\n",request.city,request.type,request.end_date.day);
    
    int ans = findRequestedCount(&request);
    //printf("ANSS: %d\n",ans);
    while(((n=write(newsockfd , &ans,sizeof(int))) == -1 ) && errno == EINTR);

    close(newsockfd);
    free(arg);
    return NULL;
}
int findRequestedCount(ClientThreadArg *request){
    if(request->city[0] == '\0'){
        // all cities will be searched
        int sum = 0;
        for (int i = 0; i < numberOfCities; i++)
        {
            sum += searchInCity(&cities[i],request);
        }
        return sum;
        
        //return 6;
    } else{
        /*int indexOfCity = -1;
        for(int i=0;i<numberOfCities;i++){
            if(strcmp(cities[i].nameOfCity,request->city)==0){
                indexOfCity = i;
                break;
            }
            
        }
        if(indexOfCity == -1){
            //perror("City is not found");
            return -1;
        }*/
        //return searchInCity(&cities[indexOfCity],request);
        Container * city = search(request->city);
        if(city == NULL || city->nameOfCity == NULL){
            return -1;
        }
        //printf("city->nameOfCity: %s\n",city->nameOfCity);
        return searchInCity(city,request);
    }
    return 0;

}
int searchInCity(Container *city,ClientThreadArg *request){
    int sum = 0;
    for (int i = 0; i < city->numberOfTransactionFiles; i++)
    {
        sum += searhInTransactionFile(&(city->transactionFiles[i]),request);
    }
    
    return sum;
}
int searhInTransactionFile(TransactionFile *tf,ClientThreadArg *request){
    int sum = 0;
    for (int i = 0; i < tf->numberOfTransactions; i++)
    {
        Date d;
        initializeDate(&d,tf->date);
        sum += isValidRequest(&d,&tf->transactions[i],request);
    }
    return sum;
}
int isValidRequest(Date * dateOfTransaction,Transaction *tr,ClientThreadArg *request){
    /*request->city;
    request->end_date;
    request->start_date;
    request->type;*/
    if(dateCompare(&request->start_date,dateOfTransaction) <=0 && dateCompare(&request->end_date,dateOfTransaction)>= 0 ){
        if(strcmp(tr->type,request->type)==0){
            /*printf("START: %d %d %d\n",request->start_date.day,request->start_date.month,request->start_date.year);
            printf("END: %d %d %d\n",request->end_date.day,request->end_date.month,request->end_date.year);
            printf("eldeki: %d %d %d\n",dateOfTransaction->day,dateOfTransaction->month,dateOfTransaction->year);
            */
            return 1;
        }
    }
    return 0;
    //if(dateCompare(tr->))
}
int getNumberOfDirectories(char const * dataSetPath){
    DIR *dir;
    struct dirent * temp;
    if ((dir = opendir(dataSetPath)) == NULL){
        perror("opendir() error");
        closedir(dir);
        return -1;
    }
    else {
        //puts("contents of root:");
        int i=0;
        while ((temp= readdir(dir)) != NULL){
            i++;
        }
        /*for (int j = 0; j < numberOfDirectories; j++)
        {
            printf("Q: %s\n",entry[j].d_name);
        }*/
        
        //qsort(entry,numberOfDirectories,sizeof(struct dirent),direntCompare);
        /*for(int x = 0;x < numberOfDirectories;x++){
            printf("AA: %d:   %s\n",x, entry[x].d_name);
            i++;
        }*/
        closedir(dir);
        return i;
    }
}
void freeHashArray(struct DataItem ** hashArray,int size){
    if(hashArray == NULL)
        return;
    for(int i = 0; i<size; i++) {
        if(hashArray[i] != NULL)
            free(hashArray[i]);
    }
    free(hashArray);
    
}
void saveToHashmap(struct DataItem ** hashArray,Container * cities){
    for (int i = 0; i < numberOfCities; i++)
    {
        insert(cities[i].nameOfCity,&cities[i]);
    }
    //display();
}
int hashCode(char* key) {
    int len = strlen(key);
    int hashcode = 0;
    for (int i = 0; i < len; i++)
    {
        hashcode += key[i];
    }
    
    return hashcode % sizeOfHashMap;
}

Container *search(char* key) {
   //get the hash 
   int hashIndex = hashCode(key);  
   //move in array until an empty 
   while(hashArray[hashIndex] != NULL) {
	
      if(strcmp(hashArray[hashIndex]->key, key) == 0)
         return hashArray[hashIndex]->data; 
			
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= sizeOfHashMap;
   }        
	
   return NULL;        
}

void insert(char* key,Container *data) {

   struct DataItem *item = (struct DataItem*) malloc(sizeof(struct DataItem));
   item->data = data;  
   strcpy(item->key,key);
   //item->key = key;

   //get the hash 
   int hashIndex = hashCode(key);

   //move in array until an empty or deleted cell
   while(hashArray[hashIndex] != NULL) {
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= sizeOfHashMap;
   }
	
   hashArray[hashIndex] = item;
}

void display() {
   int i = 0;
	
   for(i = 0; i<sizeOfHashMap; i++) {
	
      if(hashArray[i] != NULL)
         printf(" (%s,%s)",hashArray[i]->key,hashArray[i]->data->nameOfCity);
      else
         printf(" ~~ ");
   }
	
   printf("\n");
}