#include "replace_system.h"
int main(int argc,char * argv[]){

    if(argv[1] == NULL || argv[2] == NULL || argc != 3){
        printf("\nPlease enter the input in the following format:\n./hw1 ‘/^Window[sz]*/Linux/i;/close[dD]$/open/‘ inputFilePath\n");
        return 0;
    }
    char ** arr=my_split(argv[1],'/');
    
    int valid = checkInputCorrection(arr);
    if(valid){
        int numberOfReplaceOperations = my_numberOf(argv[1],';') + 1;
        programLoop(arr,argv[2],numberOfReplaceOperations);

    } else{
        printf("\nPlease enter the input in the following format:\n./hw1 ‘/^Window[sz]*/Linux/i;/close[dD]$/open/‘ inputFilePath\n");

    }
    int numberOfSeparators = my_numberOf(argv[1],'/');
    for(int i=0;i < numberOfSeparators+2;i++){
        free(arr[i]);
    }
    free(arr);

    return 0;

}