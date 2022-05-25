#include "../header/matrix.h"
#include <math.h>
#define MY_PI 3.14159265358979323846


void freeMatrix(void ** matrix,int size){
    for (int i = 0; i < size; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}
ImaginaryNumber** allocateMatrixImaginaryNumber(int n){
    ImaginaryNumber ** matrix = (ImaginaryNumber **) malloc(sizeof(ImaginaryNumber*) * n);
    if(matrix == NULL){
        perror("Matrix malloc error");
        return NULL;
    }
    for (int i = 0; i < n; i++)
    {
        matrix[i] = (ImaginaryNumber *) malloc(sizeof(ImaginaryNumber) * n);
        if(matrix[i] == NULL){
            perror("matrix malloc error");
            // free space allocated so far
            freeMatrix((void**)matrix,i);
            return NULL;
        }
    }
    return matrix;
}
int** allocateMatrix(int n){
    int ** matrix = (int **) malloc(sizeof(int*) * n);
    if(matrix == NULL){
        perror("Matrix 1 malloc error");
        return NULL;
    }
    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *) malloc(sizeof(int) * n);
        if(matrix[i] == NULL){
            perror("matrix malloc error");
            // free space allocated so far
            freeMatrix((void**)matrix,i);
            return NULL;
            
        }
    }
    return matrix;
}
void printMatrix(int ** matrix,int n){
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");
    }
}
void printMatrixImaginaryNumber(ImaginaryNumber ** matrix,int n){
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%f + j(%f),",matrix[i][j].real, matrix[i][j].imaginary);
        }
        printf("\n");
    }
}
int readMatrixFromFile(int ** matrix,int size,const char *inputFile){
    int fd = open(inputFile,O_RDONLY);
    if (fd == -1){
        perror("open");
        return -1;        
    }
    struct flock lock;
    int bytesread;
    char c;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    fcntl(fd,F_SETLKW,&lock);
    
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            while((bytesread = read(fd,&c,1)) == -1 && errno == EINTR);
            if(bytesread == -1){
                perror("read");
                return -1;
            } else if(bytesread == 0){
                fprintf(stderr,"Input file does not contaion sufficient number of characters: %s\n",inputFile);
                return -1;
            }
            matrix[i][j] = c;
        }
        
    }
    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    if(close(fd) == -1){
        perror("close");
        return -1;
    }
    return 0;
}
void subSquareMatrixMultiplication(int ** ans_matrix,int **matrix1,int **matrix2,int size,int columnStartIndex,int columnEndIndex){
    for (int i = columnStartIndex; i < columnEndIndex; i++)
    {
        for (int j = 0; j < size; j++)
        {
            int val = 0;
            for (int k = 0; k < size; k++)
            {
                val += (matrix1[j][k] * matrix2[k][i]);
            }
            ans_matrix[j][i] = val;

        }
        
    } 
}
void subSquareMatrix2DDiscreteFourierTransform(ImaginaryNumber ** out_matrix,int ** in_matrix,int columnStartIndex,int columnEndIndex,int size){
    for (int i = columnStartIndex; i < columnEndIndex; i++) // column
    {
        for (int j = 0; j < size; j++) // row
        {
            //[j][i] must be found
            double sumReal = 0.0;
            long double sumImg = 0.0;
            for (int m = 0; m < size; m++)
            {
                for (int n = 0; n < size; n++)
                {   
                    /*
                                -2PI * (j * m / M + i * n / N)
                    */
                    double firstHalf = (-2.0000 * MY_PI) /((double) size);
                    double secondHalf = (j * m) + (i * n);
                    double x = firstHalf * secondHalf; 
                    //double x = (-2.0000 * MY_PI) /((double) size) * ((j * m) + (i * n));
                    sumReal += (in_matrix[m][n] * cos(x));
                    sumImg += (in_matrix[m][n] * sin(x));
                }
            }
            out_matrix[j][i].real = sumReal;
            out_matrix[j][i].imaginary = sumImg;
        }
    } 

}
/*
 (3634888) + j(0) , (143112) + j(-29037) , (-166667) + j(12128) , (-39178) + j(-144113) , (62700) + j(0) , (-39176) + j(144107) , (-166667) + j(-12136) , (143118) + j(29029) , 
(-16784) + j(-783007) , (30820) + j(-432) , (-6009) + j(75765) , (-2942) + j(2256) , (5138) + j(-6426) , (28870) + j(-10167) , (-1606) + j(85327) , (-44184) + j(-10498) , 
(314998) + j(-43287) , (-5035) + j(9138) , (-29393) + j(-8061) , (-3021) + j(13739) , (4472) + j(8252) , (14556) + j(21858) , (-42564) + j(-2793) , (6909) + j(-23091) , 
(15976) + j(-148761) , (6512) + j(-11870) , (7763) + j(14278) , (-22080) + j(1782) , (-5964) + j(1046) , (-3418) + j(13928) , (62) + j(4722) , (2598) + j(-3281) , 
(315348) + j(0) , (3398) + j(12626) , (-31069) + j(2192) , (-13354) + j(-7569) , (-9203) + j(0) , (-13351) + j(7565) , (-31069) + j(-2193) , (3400) + j(-12624) , 
(15979) + j(148756) , (2602) + j(3283) , (61) + j(-4726) , (-3414) + j(-13930) , (-5962) + j(-1044) , (-22082) + j(-1784) , (7764) + j(-14277) , (6520) + j(11867) , 
(314998) + j(43279) , (6906) + j(23092) , (-42564) + j(2792) , (14556) + j(-21860) , (4472) + j(-8252) , (-3019) + j(-13742) , (-29393) + j(8060) , (-5035) + j(-9137) , 
(-16782) + j(783000) , (-44181) + j(10496) , (-1605) + j(-85330) , (28872) + j(10164) , (5140) + j(6423) , (-2939) + j(-2257) , (-6011) + j(-75764) , (30821) + j(433) ,
*/

/*
4105035.000000 + j(0.000000),125759.052855 + j(16662.423463),-250793.000000 + j(15128.000000),-32333.052855 + j(-149745.576537),71323.000000 + j(-0.000000),-32333.052855 + j(149745.576537),-250793.000000 + j(-15128.000000),125759.052855 + j(-16662.423463),
33485.071673 + j(-402189.901045),3693.440902 + j(-26856.247456),-1894.188309 + j(13964.730076),-10609.121425 + j(-717.922656),3241.670991 + j(1331.277995),13322.640031 + j(9583.987448),5009.433760 + j(10694.583038),3755.110606 + j(-13715.656260),
149771.000000 + j(27778.000000),20003.371840 + j(6061.671140),-8557.000000 + j(2542.000000),952.581961 + j(-2711.662474),-1269.000000 + j(5562.000000),-13581.371840 + j(5490.328860),-3573.000000 + j(1814.000000),-874.581961 + j(9399.662474),
-34267.071673 + j(-98421.901045),-5848.878575 + j(2166.077344),-4867.433760 + j(-509.416962),-6463.440902 + j(-2644.247456),-4055.670991 + j(-2708.722005),4170.889394 + j(-13199.656260),1924.188309 + j(2776.730076),-5828.640031 + j(-12452.012552),
175707.000000 + j(-0.000000),3528.867966 + j(2486.156421),-6849.000000 + j(-6816.000000),3953.132034 + j(-1897.843579),-6293.000000 + j(-0.000000),3953.132034 + j(1897.843579),-6849.000000 + j(6816.000000),3528.867966 + j(-2486.156421),
-34267.071673 + j(98421.901045),-5828.640031 + j(12452.012552),1924.188309 + j(-2776.730076),4170.889394 + j(13199.656260),-4055.670991 + j(2708.722005),-6463.440902 + j(2644.247456),-4867.433760 + j(509.416962),-5848.878575 + j(-2166.077344),
149771.000000 + j(-27778.000000),-874.581961 + j(-9399.662474),-3573.000000 + j(-1814.000000),-13581.371840 + j(-5490.328860),-1269.000000 + j(-5562.000000),952.581961 + j(2711.662474),-8557.000000 + j(-2542.000000),20003.371840 + j(-6061.671140),
33485.071673 + j(402189.901045),3755.110606 + j(13715.656260),5009.433760 + j(-10694.583038),13322.640031 + j(-9583.987448),3241.670991 + j(-1331.277995),-10609.121425 + j(717.922656),-1894.188309 + j(-13964.730076),3693.440902 + j(26856.247456),

*/