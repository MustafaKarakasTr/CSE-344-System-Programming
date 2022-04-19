#include "determinant.h"
int isInvertible(int **matrix,int size){
    /*for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");
        
    }
    */
    int result = determinant(matrix,size,0);
    //If det(A)=0 then A is not invertible
    //printf("determinant: %d\n",result);

    return (result != 0);
}
int determinant(int **arr,int size,int startIndex){
    if(startIndex == size -1 ){
        return arr[startIndex][startIndex]; // 1x1 matrix's determinant
    }
    int determinant = 0;
    for(int i=startIndex;i<size;i++){
        determinant += (arr[startIndex][i]* cofactor(arr,size,startIndex,i));
    }
    return determinant;
}
int cofactor(int ** arr,int size,int row,int column){
    int degreeOfMinusOne = row+column;
    int negativeOrPositive = (degreeOfMinusOne % 2 == 0) ? 1 : -1;
    //int negativeOrPositive =(int) Math.pow(-1.0,degreeOfMinusOne);
    int sizeOfArr = size-row-1;
    int ** temp = (int**) calloc(sizeof(int*) , sizeOfArr);
    if(temp == NULL){
        perror("Calloc Error");
        return -1;
    }
    for (int i = 0; i < sizeOfArr; i++)
    {
        temp[i] = (int*) calloc(sizeof(int) , sizeOfArr);
        if(temp[i]== NULL){
            for (int j = 0; j < i; j++)
            {
                free(temp[j]);
            }
            perror("Calloc Error");
            return -1;
            
        }
    }
    
    // new int[sizeOfArr][sizeOfArr];
    int x=0,y=0;
    for(int i = row+1;i<size;i++){
        for(int j=0;j<size;j++){
            if(j == column){
                continue;
            }else{
                temp[x][y] = arr[i][j];
                y++;
                if(y == sizeOfArr){
                    x++;
                    y = 0;
                }
            }

        }
    }
    int deterOfSub = determinant(temp,sizeOfArr,0);
    for(int i=0;i<sizeOfArr;i++){
        free(temp[i]);
    }
    free(temp);
    return negativeOrPositive * deterOfSub;
}
/*int main(){
    int ** temp = (int**) calloc(sizeof(int*) , 3);
    int sizeOfArr = 3;
    if(temp == NULL){
        perror("Calloc Error");
        return -1;
    }
    for (int i = 0; i < sizeOfArr; i++)
    {
        temp[i] = (int*) calloc(sizeof(int) , sizeOfArr);
        if(temp[i]== NULL){
            for (int j = 0; j < i; j++)
            {
                free(temp[j]);
            }
            perror("Calloc Error");
            return -1;
            
        }
    }
    srand(time(NULL));
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            temp[i][j] = rand() % 100;
            printf("%d ",temp[i][j]);
        }
        printf("\n");

    }
    printf("%d",determinant(temp,3,0));
    for (int j = 0; j < sizeOfArr; j++)
    {
        free(temp[j]);
    }
    free(temp);
}*/