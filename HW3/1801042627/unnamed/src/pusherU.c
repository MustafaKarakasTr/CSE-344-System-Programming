#include "../header/pusher.h"
#include "../header/sharad_memory.h"
/*
    argv[1]: holds ingredients name for named semaphore
    argv[2]: program name to decide which type of semaphore will be used (named/unnamed)
    argv[3]: shared memory name        
    argv[4]: isIngredient1Index isIngredient2Index isIngredient3Index isIngredient4Index %d%d%d%d, last one will the one which is assigned to 1
    argv[5]: ingredientSemaphoreNamesForChefs for isIngredient1Index
    argv[6]: ingredientSemaphoreNamesForChefs for isIngredient2Index
    argv[7]: ingredientSemaphoreNamesForChefs for isIngredient3Index
*/
int main(int argc, char const *argv[])
{
    struct stat sb;
    int indexOfIngredients[4];
    for(int i=0;i<4;i++){
        indexOfIngredients[i] = argv[4][i] - '0' + 2; // +2 is needed because shared mem holds ingredients in first 2 char indexes
    }

    
    int shm_fd = createSharedMemory(argv[2],argv[3]);
    if(shm_fd == -1){
        return -1;
    }
    /* Use shared memory object size as length argument for mmap()
    and as number of bytes to write() */
    if(fstat(shm_fd,&sb) == -1){
        if (close(shm_fd) == -1) /* 'fd' is no longer needed */
            perror("close");
        perror("fstat");
        return -1;
    }
    char *addr = mmap(NULL, sb.st_size, PROT_READ| PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap");
        if (close(shm_fd) == -1) /* 'fd' is no longer needed */
            perror("close");
        return -1;
    }
   //printf("\n");
    fflush(stdout);
    struct sigaction sa;

    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &signalHandler;
    if ((sigemptyset(&sa.sa_mask) == -1) ||  (sigaction(SIGINT,&sa, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        if (close(shm_fd) == -1) /* 'fd' is no longer needed */
            perror("close");
        return -1;
    }
    sem_t * sem_addr = (sem_t*) addr;
    sem_t* pusher_sem =  &sem_addr[2]; // its semaphore is here
    int indexOfWantedIngredient = argv[1][0] -'0';
    //printf("indexOfWantedIngredient:%d \n",indexOfWantedIngredient);
    sem_t* ingredientWaited = &sem_addr[indexOfWantedIngredient];
    //printf("ATOIII: %d\n",indexOfWantedIngredient);
    /*sem_t* ingredientWaited = sem_open(argv[1],O_CREAT, S_IRUSR | S_IWUSR,1);
    if(ingredientWaited == SEM_FAILED){
    
        perror("sem_open wholesaler");
        
        if (close(shm_fd) == -1)
            perror("close");
        return -1;
    }*/
    int ingredient1Index = hexCharToInt(argv[5][0]);
    int ingredient2Index = hexCharToInt(argv[6][0]);
    int ingredient3Index = hexCharToInt(argv[7][0]);
    //printf("INGREDIENT: %d %d %d\n",ingredient1Index,ingredient2Index,ingredient3Index);
    sem_t* ingredient1 = &sem_addr[ingredient1Index];
    sem_t* ingredient2 = &sem_addr[ingredient2Index];
    sem_t* ingredient3 = &sem_addr[ingredient3Index];


    /*sem_t* ingredient1 = sem_open(argv[5],O_CREAT, S_IRUSR | S_IWUSR,0);
    if(ingredient1 == SEM_FAILED){
        perror("sem_open wholesaler");
        //sem_close(ingredientWaited);
        if (close(shm_fd) == -1) 
            perror("close");
        return -1;
    }
    sem_t* ingredient2 = sem_open(argv[6],O_CREAT, S_IRUSR | S_IWUSR,0);
    if(ingredient2 == SEM_FAILED){
        perror("sem_open wholesaler");
        //sem_close(ingredientWaited);
        sem_close(ingredient1);


        if (close(shm_fd) == -1)
            perror("close");
        return -1;
    }
    sem_t* ingredient3 = sem_open(argv[7],O_CREAT, S_IRUSR | S_IWUSR,0);
    if(ingredient3 == SEM_FAILED){
        perror("sem_open wholesaler");
        //sem_close(ingredientWaited);
        sem_close(ingredient1);
        sem_close(ingredient2);
        if (close(shm_fd) == -1)
            perror("close");
        return -1;
    }*/
    while(1){

        sem_wait(ingredientWaited);
       //printf("%s\n",argv[1]);
        fflush(stdout);
        sem_wait(pusher_sem);
       //printf("%s\n",argv[1]);
        
        char isIngredient1 = addr[indexOfIngredients[0]];
        char isIngredient2 = addr[indexOfIngredients[1]];
        char isIngredient3 = addr[indexOfIngredients[2]];

        if(isIngredient1){
            addr[indexOfIngredients[0]] = 0;
            sem_post(ingredient1);
        } else if(isIngredient2){
            addr[indexOfIngredients[1]] = 0;
            sem_post(ingredient2);

        }else if(isIngredient3){
            addr[indexOfIngredients[2]] = 0;
            sem_post(ingredient3);

        } else{
            addr[indexOfIngredients[3]] = 1;
        }
        sem_post(pusher_sem);

        if(terminate){
            //sem_close(ingredientWaited);
            /*sem_close(ingredient1);
            sem_close(ingredient2);
            sem_close(ingredient3);
            */

            if (close(shm_fd) == -1) /* 'fd' is no longer needed */
                perror("close");
            exit(0);
        }
    }
    //sem_close(ingredientWaited);
    /*
    sem_close(ingredient1);aaaa
    sem_close(ingredient2);
    sem_close(ingredient3);
    */
    if (close(shm_fd) == -1) /* 'fd' is no longer needed */
        perror("close");
    exit(0);
}
void signalHandler(int sig){
    terminate = 1;
}
int hexCharToInt(char c){
    if(c>='0' && c<='9'){
        return (c - '0');
    } else if(c>='a' && c<='f'){
        return (c - 'a' + 10);
    }
    perror("Invalid argument");
    return -1;
}