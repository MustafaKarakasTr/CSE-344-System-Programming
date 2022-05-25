#include "../header/chef.h"
#include "../header/sharad_memory.h"
char * ingredientNames[] = {"flour","milk","sugar","walnuts"};

/*
    argv[1]: semaphore name
    argv[2]: program name to decide which type of semaphore will be used (named/unnamed)
    argv[3]: shared memory name
    argv[4]: chef number
*/
int main(int argc, char const *argv[])
{
   //printf("Chef #%d Ready: %s\n",getpid(),argv[1]);

    struct sigaction sa;
    struct stat sb;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &signalHandler;
    if ((sigemptyset(&sa.sa_mask) == -1) ||  (sigaction(SIGINT,&sa, NULL) == -1))
    {    
        perror("Failed to install SIGINT signal handler");
        return -1;
    }
    // open shared memory
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
    //sem_t * ingredient = NULL;
    char* waitedIngredient1;
    char* waitedIngredient2;
// const char * ingredientNames[] = {"flour","milk","sugar","walnuts"};
    //
    //char ingredientDecider[4] = {1}; 
    if(strcmp("a",argv[1]) == 0){
        waitedIngredient1 = ingredientNames[2];
        waitedIngredient2 = ingredientNames[3];
    } else if(strcmp("7",argv[1]) == 0){
        waitedIngredient1 = ingredientNames[1];
        waitedIngredient2 = ingredientNames[3];
    } else if(strcmp("b",argv[1]) == 0){
        waitedIngredient1 = ingredientNames[1];
        waitedIngredient2 = ingredientNames[2];
    } else if(strcmp("8",argv[1]) == 0){
        waitedIngredient1 = ingredientNames[0];
        waitedIngredient2 = ingredientNames[3];
    } else if(strcmp("c",argv[1]) == 0){
        waitedIngredient1 = ingredientNames[0];
        waitedIngredient2 = ingredientNames[2];
    }else if(strcmp("9",argv[1]) == 0){
        waitedIngredient1 = ingredientNames[0];
        waitedIngredient2 = ingredientNames[1];
    } else{
        perror("Semaphore name is not valid\n");
    }
    /*if(create_sem(&ingredient,isItNamed,argv[1],O_CREAT,0) == -1){
        perror("sem_open chef");
        if (close(shm_fd) == -1)
                perror("close");
        return -1;
    }
    if(ingredient == NULL){
        perror("HATAAAAAAAAAAAAAAA");
        if (close(shm_fd) == -1) 
            perror("close");
        return -1;
    }*/
    sem_t* sem_addr = (sem_t *) addr;

    sem_t* ingredient = &sem_addr[hexCharToInt(argv[1][0])];/*= sem_open(argv[1],O_CREAT, S_IRUSR | S_IWUSR,0);
    if(ingredient == SEM_FAILED){
        perror("sem_open chef");
        if (close(shm_fd) == -1) 
                perror("close");
        return -1;
    }*/
    // when ingredients taken, chef will make wholesaler continue using this semaphore
    sem_addr = &((sem_t*)addr)[1]; // first semaphore is here
    
    sem_t* wholesaler_sem =sem_addr;
    
    int counter=0;
    struct flock lockToWrite;
    /* lock file to write */
    memset(&lockToWrite,0,sizeof(lockToWrite));
    lockToWrite.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
    //printTime();
    //printf("Z:Server Z (%s, t=%d, r=%d) started\n",argv[4],sleepDuration,poolsize);
    char buf[100];
    char template[] = "chef%s (pid %d) is waiting for %s and %s\n";

    snprintf(buf,99,template,argv[4],getpid(),waitedIngredient1,waitedIngredient2);
    printf("%s",buf);
    fflush(stdout);

    lockToWrite.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
    while (1)
    {

       //printf("CHEF %d uyuyor %s %s\n",getpid(),argv[2],argv[3]);
        //printf("BEKLENEN: %s\n",argv[1]);
        fflush(stdout);
        
        sem_wait(ingredient);
        if(terminate){
            //sem_close(ingredient);
            //sem_close(wholesaler_sem);

            if (close(shm_fd) == -1) /* 'fd' is no longer needed */
                perror("close");
            memset(&lockToWrite,0,sizeof(lockToWrite));
            lockToWrite.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);

            printf("chef%s (pid %d) is exiting\n",argv[4],getpid());

            fflush(stdout);

            lockToWrite.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
            exit(counter);
        }
        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        printf("chef%s (pid %d) has taken the %s\n",argv[4],getpid(),waitedIngredient1);
        printf("chef%s (pid %d) has taken the %s\n",argv[4],getpid(),waitedIngredient2);
        printf("chef%s (pid %d) is preparing the dessert\n",argv[4],getpid());
        fflush(stdout);

        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);

        sleep(1);

        memset(&lockToWrite,0,sizeof(lockToWrite));
        lockToWrite.l_type = F_WRLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);

        printf("chef%s (pid %d) has delivered the dessert\n",argv[4],getpid());

        fflush(stdout);

        lockToWrite.l_type = F_UNLCK;
        fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        if(terminate){
            //sem_close(ingredient);
            //sem_close(wholesaler_sem);

            if (close(shm_fd) == -1) /* 'fd' is no longer needed */
                perror("close");
            memset(&lockToWrite,0,sizeof(lockToWrite));
            lockToWrite.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);

            printf("chef%s (pid %d) is exiting\n",argv[4],getpid());

            fflush(stdout);

            lockToWrite.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
            exit(counter);
        }
        counter++;

        //getIngredients(addr);
        //fflush(stdout);

        // post wholesaler here
        sem_post(wholesaler_sem); // ingredients taken, new ingredients can come

        
    }
    //sem_close(ingredient);
    //sem_close(wholesaler_sem);

    if (close(shm_fd) == -1) /* 'fd' is no longer needed */
        perror("close");
    return 0;
}
void signalHandler(int sig){
    terminate = 1;
}
void getIngredients(char * shr_addr){
    printf("Ingredients BUNNLAR:%c %c\n",shr_addr[0],shr_addr[1]);
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