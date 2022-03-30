#include "parentProcess.h"

sig_atomic_t signalOccured = 0;




int main(int argc,char * argv[]){
    if(argc != 5 || strcmp(argv[1],"-i")!=0 || strcmp(argv[3],"-o")!=0 ){
        fprintf(stderr,"Invalid Input, please enter in this format: ./processP -i inputFilePath -o outputFilePath\n");
        return -1;
    }
    struct sigaction newact;
    memset(&newact,0,sizeof(newact));
    newact.sa_handler = &mysighand; /* set the new handler */
    newact.sa_flags = 0;
    /* no special options */
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT,&newact, NULL) == -1))
        perror("Failed to install SIGINT signal handler");
    char * out = argv[4];
    char buf[BLKSIZE]; 
    int bytesread;
    struct flock lock;
    /* Open a file descripter to the file */
    char* file_path = argv[2];
    if(signalOccured == 1){
        return -1;
    }
    int fd = open(file_path,O_RDONLY);
    
    if(fd == -1){
        perror("open file: ");
        return 1;
    }
    if(signalOccured == 1){
        close(fd);
        return -1;
    }
    // delete file if exist
    unlink(out);

    /* Initialize the flock structure */
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    /*Place a read lock on the file. */

    fcntl(fd,F_SETLKW,&lock);
    /*file locked*/

    int processIndex = 1;
    
     for(;;){

        if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }

        memset(buf,'\0',BLKSIZE);

        while(((bytesread = read(fd,buf,BLKSIZE)) == -1) && (errno == EINTR)); // try to read until become successful

        
        if(bytesread <=0 || strlen(buf) < BLKSIZE) // end of file is reached or enough number of coordinates does not exist
        {
            break;
        }

        if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }
        int id = fork();
        
        if(id == 0){

            char* envp[] =
            { 
                    buf, // buna coordinates= eklemem lazım
                    NULL
            };

            char indexOfProcessString[20];
            sprintf(indexOfProcessString, "%d", processIndex);
            
            char * argsChild[] = {"childProgram",out,indexOfProcessString,NULL};
            //Created R_1 with (66,77,81),(110,91,34),...,(63,90,91)
            
            execve(argsChild[0],argsChild,envp);
            return 0;
        } else{
           printf("Created R_%d with ",processIndex);
           for(int i=0;i<10;i++){
               printf("(%d, %d, %d)%c",buf[i*3],buf[i*3+1],buf[i*3+2],(i!=9)?',':' '); 
           }
           printf("\n");
           processIndex++;
        }
    }
 
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);

    

    if(close(fd) == -1){
        fprintf(stderr,"close file error");
        return 1;
    }
    
    // wait for all children to finish
    int childPid;
    for(;;){
        childPid = wait(NULL);
        if(childPid == -1){
            if(errno == ECHILD){
                break;
            }
        }
    }
    printf("Reached EOF, collecting outputs from %s\n",out);
    //sleep(1);
    if(signalOccured == 1){
        unlink(out);
        return -1;
    }     
    takeOutputsFromChild(out,processIndex-1);
    if(signalOccured == 1){
        unlink(out);
        return -1;
    }  
    return 0;

}

float frobeniusNorm(Matrix * mt, int row,int column){
    float sum = 0;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            
            float temp = mt->matrix[i][j];
            sum += (temp * temp);

        }
    }   
    return sqrt(sum);
}
Matrix * createMatrix(int row,int column){
    Matrix * matrix = (Matrix*) malloc(sizeof(Matrix));
    if(matrix == NULL){
        fprintf(stderr,"Malloc Error\n");
        return NULL;
    }
    matrix->matrix = (float **) malloc(sizeof(float*) * row);
    if(matrix->matrix == NULL){
        free(matrix);
        fprintf(stderr,"Malloc Error\n");
        return NULL;
    }
    for(int i=0;i<row;i++){
        float * arr = (float*) malloc(sizeof(float) * column);
        if(arr == NULL){
            for(int j = 0;j<i;j++){
                free(matrix->matrix[j]); // free spaces allocated till this row
            }
            free(matrix);
            fprintf(stderr,"Malloc Error\n");
            return NULL;
        }
        matrix->matrix[i] = arr;
    }
    return matrix;
}
void printMatrix(Matrix* matrix,int row,int column){
    if(matrix == NULL || matrix->matrix == NULL){
        return;
    }
    printf("\n%d. matrix:\n",matrix->id);
    for(int i=0;i<row;i++){
        for (int j = 0; j < column; j++)
        {
            printf("%f ",matrix->matrix[i][j]);
        }
        printf("\n");
        
    }
}
void freeMatrix(Matrix* matrix,int row){
    for(int i=0;i<row;i++){
        free(matrix->matrix[i]);
    }
    free(matrix->matrix);
    free(matrix);
}
int takeOutputsFromChild(char *inputFile,int numberOfMatrixes){
    struct flock lock;
    /* Open a file descripter to the file */
    int fd = open(inputFile,O_RDONLY);
    
    if(fd == -1){
        fprintf(stderr,"open file error");
        return -1;
    }
    if(signalOccured == 1){
        if(close(fd) == -1){
            fprintf(stderr,"close file error");
        }
        return -1;
    }
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    /*Place a read lock on the file. */

    fcntl(fd,F_SETLKW,&lock);
    char buf[BLKSIZE];
    int bytesread=0;

    int linesReadFromFile = 0;
    int indexInMatrix = 0;
    Matrix * currMatrix = NULL;
    LinkedList * matrixList = NULL;
    int indexOfMatrix = 1;
    for(;;){
        if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }
        memset(buf,'\0',BLKSIZE);
        while(((bytesread = read(fd,buf,BLKSIZE)) == -1) && (errno == EINTR)); // try to read until become successful
        if(signalOccured == 1){
            lock.l_type = F_UNLCK;
            fcntl(fd,F_SETLKW,&lock);
            if(close(fd) == -1){
                fprintf(stderr,"close file error");
            }
            return -1;
        }

        
        if(bytesread <=0) // end of file is reached 
        {
            break;
        }

        for (int i = 0; buf[i] != '\0'; i++)
        {
            if(buf[i] == '\n'){
                int howManyCharactersReadAfterNewLine = bytesread - i - 1;
                
                if(howManyCharactersReadAfterNewLine != 0){
                    lseek(fd,-howManyCharactersReadAfterNewLine,SEEK_CUR); // put descriptor to start of newline
                }
                buf[i] = '\0';
                break;
            }
        }

        switch (linesReadFromFile % 4)
        {
            case 0:{

                int newElement = 0;
                if(currMatrix != NULL){
                    newElement = 1;
                    currMatrix = NULL;
                }
                indexOfMatrix = atoi(buf);
                indexInMatrix = 0;
                currMatrix = createMatrix(3,3);
                if(currMatrix == NULL){
                    if(matrixList != NULL){
                        freeLinkedList(matrixList); 
                    }
                }
                currMatrix->id = indexOfMatrix;
                if(newElement == 1)
                    add(matrixList,currMatrix);    
                else
                    matrixList = create(currMatrix);
                
                break;
            }
            default:{
                
                // matrix is read
                char * token = strtok(buf, ",");
                // loop through the string to extract all other tokens

                while( token != NULL  && indexInMatrix < 9) {
                    float val = atof(token);
                    
                    //printf( "QQ%fQQ\n", val ); //printing each token
                    if(currMatrix == NULL){
                        perror("CURRMATRIX IS NULL");
                    }
                    if(currMatrix->matrix == NULL){
                        perror("CURRMATRIX->matrix IS NULL");

                    }
                    currMatrix->matrix[indexInMatrix / 3][indexInMatrix % 3] = val;
                   //printf("%f ",currMatrix->matrix[indexInMatrix / 3][indexInMatrix % 3]);

                    indexInMatrix++;
                    token = strtok(NULL, ",");
                }

                break;
            }
        }

        linesReadFromFile++;
    }
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    if(close(fd) == -1){
        fprintf(stderr,"close file error");
        return 1;
    }
    if(signalOccured == 1){
        return -1;
    }
    float minDistance = FLT_MAX;
    int minIndex1 = 0;
    int minIndex2 = 0;
    float * frobenius_norms =(float*) calloc(sizeof(float),numberOfMatrixes);

    if(frobenius_norms != NULL){
        

        int indexInMatrixes=0;
        for(LinkedList * head = matrixList;head!=NULL;head=head->next){
            if(indexInMatrixes >= numberOfMatrixes){
                fprintf(stderr,"NUMBER OF MATRIXES IS NOT CORRECT\n");
                return -1;
            }
            frobenius_norms[indexInMatrixes] = frobeniusNorm((Matrix*)head->data,3,3);
            for(int i=0;i<indexInMatrixes;i++){
                float distance =frobenius_norms[indexInMatrixes]-frobenius_norms[i];
                if(distance<0)
                    distance = -distance;
                if(distance < minDistance){
                    
                    minDistance = distance;
                    minIndex1 = i;
                    minIndex2 = indexInMatrixes;
                }
            }
            indexInMatrixes++;

            
        }
    } else{
        fprintf(stderr,"calloc error");
        // next statements will free the resources, do not end the program
    }
    printf("Minimum Distance Found: %f",minDistance);
    LinkedList * nextHead;
    int indexOfMatrixInLinkedList = 0;
    for(LinkedList * head = matrixList;head!=NULL;head=nextHead){
        if(indexOfMatrixInLinkedList == minIndex1 || indexOfMatrixInLinkedList == minIndex2){
            printMatrix((Matrix*)head->data,3,3);
        }
        indexOfMatrixInLinkedList++;
        
        nextHead = head->next;
        
        freeMatrix((Matrix*)head->data,3);
    }

    freeLinkedList(matrixList);
    free(frobenius_norms);
    return 0;



}
void mysighand(int signal_number){
    signalOccured = 1;
}
