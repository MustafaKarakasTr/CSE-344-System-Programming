#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> // for memset function
#include "replace_system.h"

#define BLKSIZE 500
#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_TRUNC | O_CREAT)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)


char * case_insensitive = "i";
char * case_insensitive_2 = "i;";
char * case_sensitive = ";";

int size(char ** arr){
    if(arr == 0)
        return 0;
    int count = 0;
    while(arr[count++] != NULL);
    return count-1; // NULL element should not be counted.
}
void programLoop(char ** parameters,char * file_path,int numberOfReplaceOperations){
    int i = 0;

    /*create space for parameters*/
    replace_parameters ** rep_params = (replace_parameters**) malloc(sizeof(replace_parameters*) * numberOfReplaceOperations);
    /*error check*/
    if(rep_params == NULL){
        perror("malloc error: ");
        return;
    }
    
    //int numberOfReplaceOperations = 0;
    int replace_program_counter = 0;
    while(parameters[i] != NULL){
        char * replaced = parameters[i++];
        char * incomer = parameters[i++];
        char isCaseInsensitive = 0;
        if(parameters[i] != NULL){
            if(my_strcmp(parameters[i],case_insensitive) == 0 || my_strcmp(parameters[i],case_insensitive_2) == 0){
                isCaseInsensitive = 1;
                i++;
            } else if(my_strcmp(parameters[i],case_sensitive) == 0){
                // isCaseInsensitive is already 0
                i++;
            }
            else{ //  
                //perror("ERROR! Input error should be checked before!\n");
                return;
            }
            
        }
        replace_parameters* rp = (replace_parameters*) malloc(sizeof(replace_parameters));
        // if there is allocation error 
        if(rp == NULL){
            perror("malloc error: ");
            /* free spaces allocated before */
            for(int i=replace_program_counter-1;i>=0;i--){
                free(rep_params[i]);
            }
            free(rep_params);
            /* Give back control of program to main function to close program. */
            return;
        }
        /*place inputs to struct */
        rp->replaced = replaced;
        rp->incomer = incomer;
        rp->isCaseInsensitive = isCaseInsensitive;

        /*save struct to array*/
        rep_params[replace_program_counter] = rp;
        
        replace_program_counter++;
    }

    /* parameters are ready, start reading from file and keep replacing */
    replace_program(rep_params,numberOfReplaceOperations,file_path);
    for (int i = 0; i < replace_program_counter; i++)
    {
        free(rep_params[i]);
    }
    
    free(rep_params);
}
int checkInputCorrection(char ** parameters){
    if (parameters == NULL || parameters[0] == NULL || parameters[1] == NULL){
        perror("Invalid input! Please check input format");
        return 0;
    }
    int isValid = checkSquareBracket(parameters[0]);
    if(!isValid){
        return 0;
    }
    isValid = checkAsterixOperator(parameters[0]);
    if(!isValid){
        return 0;
    }
    isValid = checkOtherOperators(parameters[0]);
    if(!isValid){
        return 0;
    }
    for(int i=2;parameters[i]!= NULL;i++){
        if(my_strcmp(parameters[i],case_insensitive) == 0){
            if(parameters[i+1] != NULL){
                fprintf(stderr,"; is needed to insert following element. Input is not correct\n");
                return 0;
            }
        } else if(my_strcmp(parameters[i],case_insensitive_2) == 0){
            if(parameters[i+1] == NULL){
                fprintf(stderr,"There is ';' but there is no following element. Input is not correct\n");
                return 0;
            }
        } else if(my_strcmp(parameters[i],case_sensitive) == 0){
            if(parameters[i+1] == NULL){
                fprintf(stderr,"There is ';' but there is no following element. Input is not correct\n");
                return 0;
            }
        } 
        else{
            perror("Invalid input! Please check input format");
            return 0;
        }
        i++;
        // first argument does not have to exist
        if(parameters[i] == NULL){
            break;
        } else{
            
            int isValid = checkSquareBracket(parameters[i]);
            if(!isValid){
                return 0;
            }
            
            isValid = checkAsterixOperator(parameters[i]);
            if(!isValid){
                return 0;
            }
            isValid = checkOtherOperators(parameters[0]);
            if(!isValid){
                return 0;
            }
        }
        i++;
        if(parameters[i] == NULL){
            // second argument does not exist
            fprintf(stderr,"There is no following element. Input is not correct\n");
            return 0;
        }
    }
    return 1;
}
/*
    if the str is not valid in concern with [], returns 0
    else returns 1
*/
int checkSquareBracket(char * str){
    for(int j = 0;str[j] != '\0';j++){
        // closed before opening
        if(str[j] == ']'){
            fprintf(stderr,"Invalid Input, ] is encountered before relative [ ");
            return 0;
        } else if(str[j] == '['){
            j++;
            char flag = 1;
            for(;str[j] != '\0';j++){
                if(str[j] == ']'){
                    flag = 0;
                    break;
                }
            }
            if(flag){ // ] is not encountered
                fprintf(stderr,"Invalid Input, There is no ] associated with [");
                return 0;
            }
        }
    }
    return 1;
}
int checkAsterixOperator(char* str){
    if(str == NULL || str[0] == '\0'){
        return 1; // there is no * operator, this function does not care about other situations
    }
    // asterix operator does not follow any element
    if(str[0] == '*'){
        fprintf(stderr,"* operator does not follow any element\n");
        return 0;
    }
    if(str[0] != '\0' && str[1] == '*' && str[2] == '\0'){
        // "a*" is not valid string, because * operator will try to change "" string with given element
        fprintf(stderr,"Invalid Input. Empty string cannot be replaced by any string.\n");
        return 0;
    }
    return 1;
}
// check $ and ^ operator
int checkOtherOperators(char * str){
    if(str == NULL || str[0] == '\0'){
        return 1;
    }
    for (int i = 0; str[i] != '\0'; i++)
    {
        if(str[i] == '^' && i != 0) // '^' operator only valid if it is in the first index
            return 0;
        
        if(str[i] == '$' && str[i+1] != '\0') // '$' operator only valid if it is in the last index
            return 0;
    }
    return 1;
    
    
}
int replace_program(replace_parameters** rep_params,int rep_params_size,char * file_path){
    char buf[BLKSIZE]; 
    int bytesread;
    /*int byteswritten;
    int totalbytes;*/
    
    struct flock lock;
    /* Open a file descripter to the file */
    int fd = open(file_path,O_RDONLY);
    
    if(fd == -1){
        perror("open file: ");
        return 1;
    }
    /* Initialize the flock structure */
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    /*Place a read lock on the file. */

    fcntl(fd,F_SETLKW,&lock);
    /*file locked*/

    char* replacedString = NULL;
    char first_readed = 1;
    char template[] = "/tmp/tempFileXXXXXX";
    int tempFileFd = mkstemp(template);
    if(tempFileFd == -1){
        perror("Temp file could not be opened.\n");
        return -1;
    }
    for(;;){
        memset(buf,'\0',BLKSIZE);
        while(((bytesread = read(fd,buf,BLKSIZE-1)) == -1) && (errno == EINTR)); // try to read until become successful
        buf[bytesread] = '\0';


        if(bytesread <=0) // end of file is reached
            break;


        // each replace operation has potential to change str address. Check the description of replace function in replace_system.h file.
        replacedString = buf;
        
        for(int j=0;j<rep_params_size;j++){
            // for each parameter, replace operation will be done in this buffer
            char *temp  = replace(rep_params[j],replacedString,my_strlen(replacedString),first_readed);
            if(temp != replacedString && replacedString != buf){
                free(replacedString);
            }
            replacedString = temp;
        }
        while(((write(tempFileFd,replacedString,strlen(replacedString))) == -1) && (errno == EINTR));
        first_readed = 0;
    }
    if(replacedString != buf){
        free(replacedString);
    }
    if(-1 == lseek(tempFileFd,0,SEEK_SET))
    {
        fprintf(stderr,"\n lseek failed with error [%s]\n",strerror(errno));    
        return 1;
    }
    /* Release the lock. */
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    
    if(close(fd) == -1){
        perror("close");
        return 1;
    }

   struct flock lockToWrite;
    if((fd = open(file_path,WRITE_FLAGS,WRITE_PERMS)) == -1){
        perror("Failed to put changes to file");
        return 1;
    }
    /* lock file to write */
    memset(&lockToWrite,0,sizeof(lockToWrite));
    lockToWrite.l_type = F_WRLCK;
    fcntl(fd,F_SETLKW,&lockToWrite);

    copyfile(tempFileFd,fd);
    
    // unlock
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);

    if(close(fd) == -1){
        perror("close");
        return 1;
    }

    unlink(template);
    if(close(tempFileFd) == -1){
        perror("Temp file could not be closed.\n");
        return -1;
    }
    return 0;
}

char* replace(replace_parameters* rep_param,char * str,int str_size,char bool_first_readed){
    int start=0;
    char * changedStr=NULL;
    // returns indexes of occurences of searched string in str
    int **indexes = startandEndIndexes(str,str_size,rep_param->replaced,rep_param->isCaseInsensitive,bool_first_readed);
    /*
    [15,5,1,-1]
    at the end of first array, there is -1 to show that array ends.
    from higher index to lower, we will replace words which start at given indexes in first array and end at given indexes at the second array
    */
    if(indexes == NULL){
        return str;
    }
    int* startIndexes = indexes[0];
    int* endIndexes = indexes[1];

    int coun = 0;
    changedStr = str;
    while(startIndexes[coun] != -1){
        start = startIndexes[coun];
        int end = endIndexes[coun];
        char * temp = strReplace(changedStr,start,end,rep_param->incomer); 
        if(temp != changedStr && changedStr != str){
            free(changedStr);
        }
        changedStr = temp;
        coun++;
    }
    free(indexes[0]);
    free(indexes[1]);
    free(indexes);
    /*if(str != changedStr){
        free(str);
    }*/
    return changedStr;
}

int copyfile(int fromfd,int tofd){
    char * bp;
    char buf[BLKSIZE];
    int bytesread;
    int byteswritten = 0;
    int totalbytes = 0;
    
    for(   ;   ;   ){
        while(((bytesread = read(fromfd, buf,BLKSIZE-1)) == -1)&&
            (errno == EINTR));
        if(bytesread<=0)
            break;
        //buf[BLKSIZE-1] = '\0';
        //buf[bytesread] = '\0';
        bp = buf;
        while(bytesread > 0){
            while(((byteswritten = write(tofd,bp,bytesread)) == -1) &&
                (errno = EINTR));
            if(byteswritten < 0){
                break;
            }
            totalbytes += byteswritten;
            bytesread -= byteswritten;
            bp += byteswritten;
        }
        if(byteswritten == -1)
            break;
    }
    return totalbytes;
}
