#include "childProgram.h"

sig_atomic_t signalOccured = 0;



int main(int argc,char* argv[]){

    if(argc != 3){
        fprintf(stderr,"Not enough information is passed to the process\n");
        return -1;
    }
    char * out = argv[1]; // output file name is assigned
    extern char** environ;
    if(environ == NULL || environ[0] == NULL){
        fprintf(stderr,"Environment variables are not sufficient\n");
        return -1;
    }
    struct sigaction newact;
    memset(&newact,0,sizeof(newact));
    newact.sa_handler = &mysighand; /* set the new handler */
    newact.sa_flags = 0;
    /* no special options */
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT,&newact, NULL) == -1))
        perror("Failed to install SIGINT signal handler");


    if(signalOccured == 1){
        return -1;
    }
    char* xValues= (char*) malloc(sizeof(char) * NUMBER_OF_POINTS);
    if(xValues == NULL){
        perror("malloc error:");
        return -1;
    }

    char* yValues= (char*) malloc(sizeof(char) * NUMBER_OF_POINTS);
    if(yValues == NULL){
        perror("malloc error:");
        free(xValues);
        return -1;
    }

    char* zValues= (char*) malloc(sizeof(char) * NUMBER_OF_POINTS);
    if(zValues == NULL){
        perror("malloc error:");
        free(xValues);
        free(yValues);

        return -1;
    }
    if(signalOccured == 1){
        free(xValues);
        free(yValues);
        free(zValues);
        return -1;
    }
    char * coordinates = environ[0];
    for (int i = 0,j = 0; j < NUMBER_OF_POINTS;j++)
    {
        xValues[j] = coordinates[i++]; 
        yValues[j] = coordinates[i++];
        zValues[j] = coordinates[i++];
    }
    writeToFile(out,xValues,yValues,zValues,argv[2]);
    free(xValues);
    free(yValues);
    free(zValues);
    return 0;
}
int writeToFile(char* outFile,char* xValues,char* yValues,char* zValues,char* indexOfProcess){
    struct flock lock;
    int fd = open(outFile,WRITE_FLAGS,WRITE_PERMS);
    
    if(fd == -1){
        perror("open file: ");
        return 1;
    }
    if(signalOccured == 1){
        if(close(fd) == -1){
            fprintf(stderr,"close file error");
        }
        return -1;
    }
    double ** covarianceM = (double**)malloc(sizeof(double*) * 3);
    if(covarianceM == NULL){
        fprintf(stderr,"malloc error");
        if(close(fd) == -1){
            fprintf(stderr,"close file error");
        }
        return -1;
    }
    double row1[3],row2[3],row3[3];

    covarianceM[0] = row1;
    covarianceM[1] = row2;
    covarianceM[2] = row3;
    /* Initialize the flock structure */
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    /*Place a read lock on the file. */

    fcntl(fd,F_SETLKW,&lock);
    
    covariance_matrix(covarianceM,xValues,yValues,zValues,NUMBER_OF_POINTS);
    
    if(signalOccured == 1){
        lock.l_type = F_UNLCK;
        fcntl(fd,F_SETLKW,&lock);
        if(close(fd) == -1){
            fprintf(stderr,"close file error");
        }
        free(covarianceM);
        return -1;
    }
    char comma = ',';
    char newLine = '\n';
    char doubleStr[MAX];

    while(((write(fd,indexOfProcess,strlen(indexOfProcess))) == -1) && (errno == EINTR));
    while(((write(fd,&newLine,sizeof(char))) == -1) && (errno == EINTR));

    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            sprintf(doubleStr,"%.3lf",covarianceM[i][j]); // convert double to string
            while(((write(fd,doubleStr,strlen(doubleStr))) == -1) && (errno == EINTR));
            while(((write(fd,&comma,sizeof(char))) == -1) && (errno == EINTR));
        }
        while(((write(fd,&newLine,sizeof(char))) == -1) && (errno == EINTR));
    }
    
    
    free(covarianceM);
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    
    if(close(fd) == -1){
        fprintf(stderr,"close file error");
        return 1;
    }
    if(signalOccured == 1){
        return -1;
    }
    return 0;
}

void mysighand(int signal_number){
    signalOccured = 1;
}
double mean(char *arr,int size){
    double sum = 0;
    for(int i=0;i<size;i++){
        sum += arr[i];
    }
    return (sum /(double)size);
}

double covariance(char arr1[], char arr2[], int size){
    double sum = 0;
    double meanArr1 = mean(arr1, size);
    double meanArr2 = mean(arr2, size);

    for(int i = 0; i < size; i++)
    sum = sum + (arr1[i] - meanArr1) * (arr2[i] - meanArr2);
    //printf("fonk: %f ",sum / ((double)size ));
    return sum / ((double)size );
}

void covariance_matrix(double **matrix,char arr1[],char arr2[],char arr3[],int n){
    char *arrays[] = {arr1,arr2,arr3};
    for(int i=0;i<3;i++){
        for(int j=i;j<3;j++){
            double temp = covariance(arrays[i],arrays[j],n);
            // it is symetric
            matrix[i][j] = temp;
            matrix[j][i] = temp;
        }
    }
}