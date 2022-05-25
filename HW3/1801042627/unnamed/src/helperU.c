#include "../header/wholesaler.h"
#include "../header/sharad_memory.h"
const char *ingredientSemaphoreNames[]= {"/MILK","/FLOUR","/WALNUTS","/SUGAR"};
// argv[3] holds semaphore name used to read access
int main(int argc, char const *argv[]) // argv[1] holds ./hw3named/unnamed //
{
   //printf("Wholesaler #%d Ready:\n",getpid());
    struct sigaction sa;
    struct stat sb;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &signalHandler;
    if ((sigemptyset(&sa.sa_mask) == -1) ||  (sigaction(SIGINT,&sa, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    int shm_fd = createSharedMemory(argv[1],argv[2]);
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
    // open semaphore to take ingredients
    //sem_t *sem_addr = &((sem_t*)addr)[1]; // first semaphore is here 
    //sem_t* wholesaler_sem = sem_addr;
    /*sem_t* wholesaler_sem = sem_open(SEMFORWHOLESALER,O_CREAT, S_IRUSR | S_IWUSR,0); // it will be 
    if(wholesaler_sem == SEM_FAILED){
        perror("sem_open wholesaler");
        if (close(shm_fd) == -1) 
            perror("close");
        return -1;
    }*/

    sem_t* ingredientsSem[4];
    for (int i = 0; i < 4; i++)
    {
        /*ingredientsSem[i] = sem_open(ingredientSemaphoreNames[i],O_CREAT, S_IRUSR | S_IWUSR,0); // it will be 
        if(ingredientsSem[i] == SEM_FAILED){
            perror("sem_open wholesaler");
            if (close(shm_fd) == -1)
                perror("close");
            for (int j = 0; j < i; j++)
            {
                sem_close(ingredientsSem[j]);
            }
            //sem_close(wholesaler_sem);
            
            return -1;
        }*/
        ingredientsSem[i] = &((sem_t*)addr)[i+3];
    }
    //printf("ARGV[3] : %s\n",argv[3]);
    sem_t* helper_can_read = &((sem_t*)addr)[13];
    //= sem_open(argv[3],O_CREAT, S_IRUSR | S_IWUSR,0); // it will be 
    /*if(helper_can_read == SEM_FAILED){
        perror("sem_open wholesaler");
        if (close(shm_fd) == -1) 
            perror("close");
        sem_close(wholesaler_sem);
        for (int i = 0; i < 4; i++)
        {
            sem_close(ingredientsSem[i]);
        }
        
        return -1;
    }*/
    while (1)
    {
        //sleep(1);
        if(terminate){
            if (close(shm_fd) == -1) /* 'fd' is no longer needed */
                perror("close");
            //sem_close(wholesaler_sem);
            //sem_close(helper_can_read);
            /*for (int i = 0; i < 4; i++)
            {
                sem_close(ingredientsSem[i]);
            }*/
            // main program will unlink wholesaler_sem
            exit(0);
        }

        sem_wait(helper_can_read);

        if(terminate){
            if (close(shm_fd) == -1) /* 'fd' is no longer needed */
                perror("close");
            //sem_close(wholesaler_sem);
            //sem_close(helper_can_read);
            /*for (int i = 0; i < 4; i++)
            {
                sem_close(ingredientsSem[i]);
            }*/
            // main program will unlink wholesaler_sem
            exit(0);
        }

        
        // read from shared memory
        //printf("wholesaler: %c %c\n",addr[0],addr[1]);
        char is_Ingredient[4] ={0};
        //char isMilk=0,isFlour=0 ,isWalnuts =0,isSugar=0;
        
        is_Ingredient[MILK]      = (addr[0] == 'M' || addr[1] == 'M');
        is_Ingredient[FLOUR]     = (addr[0] == 'F' || addr[1] == 'F');
        is_Ingredient[WALNUTS]   = (addr[0] == 'W' || addr[1] == 'W');
        is_Ingredient[SUGAR]     = (addr[0] == 'S' || addr[1] == 'S');
        
        /*for(int i=0;i<4;i++){
            addr[2+i] = is_Ingredient[i];
            ///printf("%d,",is_Ingredient[i]);
        }*/
        //printf("\n");
        // in shared memory, ingredient'ss first letters written in order
        /*char * charAddr = (char*)addr;
        if(charAddr[0] > charAddr[1]){
            char temp = charAddr[0];
            charAddr[0] = charAddr[1];
            charAddr[1] = temp;
        }*/
       //printf("AAAAAA: %c%c ",(char*)addr[0],(char*)addr[1]);
        for (int i = 2; i < 6; i++)
        {
           //printf("%d,",(char*)addr[i]);
        }
       //printf("\n");


        // post semaphores for waiting pushers, only 2 of them is 1

        for (int i = 0; i < 4; i++)
        {
            if(is_Ingredient[i] == 1){
                sem_post(ingredientsSem[i]);
            }
        }
        
        
        //sem_post(wholesaler_sem); // wholesaler has taken ingredients, new ingredients can be saved to shared memory
    }
    return 0;
}
void signalHandler(int sig){
    terminate = 1;
}

