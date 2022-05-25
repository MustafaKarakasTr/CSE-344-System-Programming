#include "../header/main.h"
#include "../header/sharad_memory.h"
char *ingredientSemaphoreNames[]= {"/MILK","/FLOUR","/WALNUTS","/SUGAR"};
char *ingredientSemaphoreNamesForChefs[]= //{"/MILK2","/FLOUR2","/WALNUTS2","/SUGAR2"};
{"/FS",
"/MS",
"/SW",
"/FM",
"/FW",
"/MW"};
char * ingredientNames[] = {"flour","milk","sugar","walnuts"};
// ./hw3named -i inputFilePath -n name
/*int main2(){
    if(sem_unlink(SEMFORWHOLESALER) == -1)
        perror("sem_unlink SEMFORWHOLESALER");
    if(sem_unlink(HELPER_CAN_READ) == -1)
        perror("sem_unlink HELPER_CAN_READ");
}*/
int main(int argc, char const *argv[])
{   
    /*for (int i = 0; i < 4; i++)
    {
        printf("%s\n",ingredientNames[i]);
    }*/
    
    if(argc != 5){
        perror("Invalid arguments");
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
    //createSharedMemory(argv[4]);
    const char * inputFilePath = argv[2];
    char *sharedMemoryName  = SHARED_MEM_NAME;
    //(argc > 4) ? argv[4] : NULL;

    int chefsReady = forkChefs(sharedMemoryName,argv[0]);
    if(chefsReady == -1){
        waitForChildren();
        return -1;
    }
    struct flock lockToWrite;
    /* lock file to write */
   
    /*int fd = open(inputFilePath,O_RDONLY);
    struct flock lock;

    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    fcntl(fd,F_SETLKW,&lock);
    char arr[3];
    int i=0;
   //printf("AAAAAAAAAA\n");
    int bytesread = 0;
    memset(arr,'\0',3);

    while ((bytesread = read(fd,arr,sizeof(char)*3)) > 0)
    {
       //printf("%c%c%c",arr[0],arr[1],arr[2]);
        if(!isValid(arr,2)){
            perror("Invalid input");
            waitForChildren();
            return -1;
        }
        memset(arr,'\0',3);
       //printf("AAAAAAAAa\n");
    }*/
    
    //int bytresread = read(fd,arr,10*sizeof(int));
    /*printf("%d byte okundu.\n",bytresread);
    for (int i = 0; i < 10; i++)
    {
       //printf("%d\n",arr[i]);
    }*/
    /*lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    if(close(fd)==-1){
        perror("close fd");
    }*/


    // create shared memory

    
   //printf("sharedMemoryName: %s\n",sharedMemoryName);
    int sharedFd = shm_open(sharedMemoryName, O_CREAT|/*O_EXCL|*/O_RDWR/*|FD_CLOEXEC*/, S_IRUSR | S_IWUSR);
    if(sharedFd == -1){
        waitForChildren();
        return -1;
    }
    /*if((sharedFd = createSharedMemory(argv[0],sharedMemoryName)) == -1){
        waitForChildren();
        return -1;
    }*/

    int requiredSize = sizeof(char) * 6;
    if (ftruncate(sharedFd, requiredSize) == -1)
    {
        perror("ftruncate");
        waitForChildren();
        return -1;
    }
    //sharedFd = shm_open(sharedMemoryName, O_CREAT|O_EXCL|O_RDWR/*|FD_CLOEXEC*/, S_IRUSR | S_IWUSR);
    /*if (sharedFd == -1)
    {
        perror("shm_open");
        return -1;
    }

    int requiredSize = sizeof(char) * 2;
    if (ftruncate(fd, requiredSize) == -1)           
    {
        perror("ftruncate");
        return -1;
    }*/
    void* addr = mmap(NULL, requiredSize, PROT_READ | PROT_WRITE , MAP_SHARED, sharedFd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap");
        waitForChildren();
        return -1;
    }
    // create semaphore for wholesaler and pushers
     // initially its value is one so wholesaler can put ingredients to shared memory
    sem_unlink(SEMFORWHOLESALER);
    sem_t* wholesaler_sem = sem_open(SEMFORWHOLESALER,O_CREAT | O_EXCL, S_IRUSR | S_IWUSR,1);
    if(wholesaler_sem == SEM_FAILED){
        perror("sem_open wholesaler1");
        waitForChildren();
        return -1;
    }
    // initially it is 1 because it is used to make only one pusher work at the same time
    sem_unlink(PUSHER_SEM);
    sem_t* pusher_sem = sem_open(PUSHER_SEM,O_CREAT | O_EXCL, S_IRUSR | S_IWUSR,1);
    if(pusher_sem == SEM_FAILED){
        perror("sem_open PUSHER_SEM");
        waitForChildren();
        return -1;
    }
    sem_t *semaphoreForPushers[NUMBEROFINGREDIENTS];
    sem_t *semaphoreForChefs[NUMBER_OF_CHEFS];

    
    if(createSemaphores(semaphoreForPushers,semaphoreForChefs) == -1)
    {
        perror("Could not create semaphores");
        waitForChildren();
        return -1;
    }
    // -n name argument is used
    char argv0[strlen(argv[0])+2];
    memset(argv0,'\0',strlen(argv0));

    memcpy(argv0,argv[0],strlen(argv[0])+1 );

    char argv4[strlen(argv[4])+2];
    memset(argv4,'\0',strlen(argv4));
    memcpy(argv4,argv[4],strlen(argv[4]) +1);

    sem_unlink(argv4);
    sem_t* helper_can_read = sem_open(argv4,O_CREAT, S_IRUSR | S_IWUSR,0);
    if(helper_can_read == SEM_FAILED){
        perror("sem_open argument name");
        waitForChildren();
        unlink_semaphores(semaphoreForPushers,semaphoreForChefs);
        return -1;
    }
    //perror("1");

    // removing const 


    int wholesaler_id = forkWholesaler(sharedMemoryName,argv0,argv4);
    if(wholesaler_id == -1){
        waitForChildren();
        unlink_semaphores(semaphoreForPushers,semaphoreForChefs);
        return -1;
    }
    int pushersForked = forkPushers(sharedMemoryName,argv0);
    if(pushersForked == -1){
        waitForChildren();
        unlink_semaphores(semaphoreForPushers,semaphoreForChefs);
        return -1;
        
    }
    int fd = open(inputFilePath,O_RDONLY);
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    fcntl(fd,F_SETLKW,&lock);
    int delivered = 0;
    while(1){
        //sleep(1);
        
        char arr[3];
        //printf("AAAAAAAAAA\n");
        int bytesread = 0;
        memset(arr,'\0',3);
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_RDLCK;
        fcntl(fd,F_SETLKW,&lock);
        char * sharedAddress = addr;
        /*printf("AAAAAA: %c%c%c",sharedAddress[0],sharedAddress[1],arr[2]);
        for (int i = 2; i < 6; i++)
            {
               //printf("%d,",sharedAddress[i]);
            }
           //printf("\n");*/
        sem_wait(wholesaler_sem); // when it is read, this semaphore will be 'post'ed
        if(delivered){
            memset(&lockToWrite,0,sizeof(lockToWrite));
            lockToWrite.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
            //printTime();
            //printf("Z:Server Z (%s, t=%d, r=%d) started\n",argv[4],sleepDuration,poolsize);

            printf("the wholesaler (pid %d) has obtained the dessert and left\n",getpid());
            
            fflush(stdout);
            lockToWrite.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
        } else{
            delivered = 1;
        }
        if ((bytesread = read(fd,arr,sizeof(char)*3)) > 0)
        {
            //printf("%c%c%c",arr[0],arr[1],arr[2]);

            
            if(!isValid(arr,2)){
                perror("Invalid input");
                removeSharedMemory(argv0,sharedFd,sharedMemoryName); // even if it returns -1, there is nothing to do, error message is//printed
                
                waitForChildren();
                unlink_semaphores(semaphoreForPushers,semaphoreForChefs);
                if(close(fd)==-1){
                    perror("close fd");
                }
                if(sem_close(wholesaler_sem) == -1)
                    perror("sem_close wholesaler_sem");
                if(sem_unlink(SEMFORWHOLESALER) == -1)
                    perror("sem_unlink wholesaler_sem");

                if(sem_close(helper_can_read) == -1)
                    perror("sem_close wholesaler_sem");
                if(sem_unlink(argv4) == -1)
                    perror("sem_unlink wholesaler_sem");
                
                if(sem_close(pusher_sem) == -1)
                    perror("sem_close pusher_sem");
                if(sem_unlink(PUSHER_SEM) == -1)
                    perror("sem_unlink wholesaler_sem");
                return -1;
            }
            // critical region starts
            // down
            /*for (int i = 2; i < 6; i++)
            {
               //printf("%d,",sharedAddress[i]);
            }
           //printf("\n");*/
            // in shared memory, ingredient'ss first letters written in order
            if(arr[0] > arr[1]){
                char temp = arr[0];
                arr[0] = arr[1];
                arr[1] = temp;
            }

            strncpy(sharedAddress,arr,2);
            
            int ingredient1;
            int ingredient2;

            if(arr[0] == 'F'){
                ingredient1 =0;
            } else if(arr[0] == 'M'){
                ingredient1 = 1;
            } else if(arr[0] == 'S'){
                ingredient1 = 2;
            } 
            if(arr[1] == 'W'){
                ingredient2 = 3;
            } else if(arr[1] == 'M'){
                ingredient2 = 1;
            } else if(arr[1] == 'S'){
                ingredient2 = 2;
            } 
            //printf("Ingredients: %s %s\n",ingredientNames[ingredient1],ingredientNames[ingredient2]);
            
            memset(&lockToWrite,0,sizeof(lockToWrite));
            lockToWrite.l_type = F_WRLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
            //printTime();
            //printf("Z:Server Z (%s, t=%d, r=%d) started\n",argv[4],sleepDuration,poolsize);
            char buf[100];
            char template[] = "the wholesaler (pid %d) delivers %s and %s\n";

            snprintf(buf,99,template,getpid(),ingredientNames[ingredient1],ingredientNames[ingredient2]);
            printf("%s",buf);
            printf("the wholesaler (pid %d) is waiting for the dessert\n",getpid());
            
            fflush(stdout);

            lockToWrite.l_type = F_UNLCK;
            fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
            sem_post(helper_can_read);

            //post
            // critical region ends
            
            //printf("%c%c%c\n",sharedAddress[0],sharedAddress[1],arr[2]);

            //printf("AAAAAAAAa\n");
        } else{
            if(bytesread == -1){
                perror("read");
            }
            terminate = 1;
        }
        lock.l_type = F_UNLCK;
        fcntl(fd,F_SETLKW,&lock);

        if(terminate){
            /*if (close(sharedFd) == -1) 
                 perror("close");*/
                

            // if(shm_unlink())
           //printf("sharedMemoryName: %s\n",sharedMemoryName);
            removeSharedMemory(argv0,sharedFd,sharedMemoryName); // even if it returns -1, there is nothing to do, error message is//printed
            if(close(fd)==-1){
                perror("close fd");
            }
            waitForChildren();
            unlink_semaphores(semaphoreForPushers,semaphoreForChefs);
            if(sem_close(wholesaler_sem) == -1)
                perror("sem_close wholesaler_sem");
            if(sem_unlink(SEMFORWHOLESALER) == -1)
            perror("sem_unlink wholesaler_sem");
            if(sem_close(helper_can_read) == -1)
                perror("sem_close wholesaler_sem");
            if(sem_unlink(argv4) == -1)
                perror("sem_unlink wholesaler_sem");

            if(sem_close(pusher_sem) == -1)
                perror("sem_close pusher_sem");
            if(sem_unlink(PUSHER_SEM) == -1)
                perror("sem_unlink wholesaler_sem");
            return -1;
        }
    }
    removeSharedMemory(argv0,sharedFd,sharedMemoryName); // even if it returns -1, there is nothing to do, error message is//printed
    if(close(fd)==-1){
        perror("close fd");
    }
    waitForChildren();
    unlink_semaphores(semaphoreForPushers,semaphoreForChefs);
    if(sem_close(wholesaler_sem) == -1)
        perror("sem_close wholesaler_sem");
    if(sem_unlink(SEMFORWHOLESALER) == -1)
    perror("sem_unlink wholesaler_sem");
    if(sem_close(helper_can_read) == -1)
        perror("sem_close wholesaler_sem");
    if(sem_unlink(argv4) == -1)
        perror("sem_unlink wholesaler_sem");
    if(sem_close(pusher_sem) == -1)
        perror("sem_close pusher_sem");
    if(sem_unlink(PUSHER_SEM) == -1)
        perror("sem_unlink wholesaler_sem");
    return 0;
}
int forkWholesaler(char * sharedMemoryName,char* programName,char* sem_name)
{
    int id_wholesaler = fork();
    if(id_wholesaler == -1){
        perror("fork error");
        return -1;
    } else if(id_wholesaler == 0){
        // create wholesaler to bring ingredients
        // argv[4] holds name of semaphore used for read permission
        char * argv[] = {"./helper",programName,sharedMemoryName,sem_name,NULL};
        execv(argv[0],argv);
    } else{
        
        childProcesses[activeChildProcesses++] = id_wholesaler;
    }
    return id_wholesaler;
}
int forkChefs(char * sharedMemoryName,const char* programName){
    /*int id_wholesaler = fork();
    if(id_wholesaler == -1){
        perror("fork error");
        return -1;
    } else if(id_wholesaler == 0){
        // create wholesaler to bring ingredients
        char * argv[] = {"./wholesaler",argv[0],NULL};
        execv(argv[0],argv);
    } else{
        childProcesses[NUMBER_OF_CHEFS] = id_wholesaler;
    }*/
    char chefNumber[2] = "1";
    for(int i=0;i<NUMBER_OF_CHEFS;i++){
        int id = fork();
        if(id == -1){
            perror("Fork Error");
            return -1;
        }
        if(id == 0){
            /*
            argv[1]: semaphore name
            argv[2]: program name to decide which type of semaphore will be used (named/unnamed)
            argv[3]: shared memory name
            argv[4]: chef number

            */
            char * argv[] = {"./chef",ingredientSemaphoreNamesForChefs[i],programName,sharedMemoryName,chefNumber,NULL};
            execv(argv[0],argv);
        } else{
            childProcesses[activeChildProcesses++] = id;
            chefNumber[0]++;
        }
    }
    return 0;

}
int isValid(char * str,int size){
    for (int i = 0; i < size; i++)
    {
        char* ans = index(ingredients,str[i]);
        if(ans == NULL){
            return 0;
        }
    }
    return 1;
}
void unlink_semaphores(sem_t *sem_arr_pusher[],sem_t *sem_arr_chef[]){
    for(int i=0;i<NUMBEROFINGREDIENTS;i++){

        if(sem_close(sem_arr_pusher[i]) == -1)
            perror("sem_close wholesaler_sem");
        if(sem_unlink(ingredientSemaphoreNames[i]) == -1)
            perror("sem_unlink wholesaler_sem");

        
    }
    for (int i = 0; i < NUMBER_OF_CHEFS; i++)
    {
        if(sem_close(sem_arr_chef[i]) == -1)
            perror("sem_close wholesaler_sem");
        if(sem_unlink(ingredientSemaphoreNamesForChefs[i]) == -1)
            perror("sem_unlink wholesaler_sem");
    }
    
}

void waitForChildren(){
    int sum = 0;
    struct flock lockToWrite;
    char errorFlag = 0;
    for(int i=0;i<activeChildProcesses;i++){
        kill(childProcesses[i],SIGINT);
    }
    for(int i=0;i<activeChildProcesses;i++){
        int status;
        wait(&status);
        int returnValueOfChild = WEXITSTATUS(status);
        
        //printf("id :%d result: %d\n",id,returnValueOfChild);
        sum += returnValueOfChild;
        if(returnValueOfChild == -1){
            errorFlag = 1;
        }
    }
    if(errorFlag){
        perror("Something went wrong.\n");
    }
    memset(&lockToWrite,0,sizeof(lockToWrite));
    lockToWrite.l_type = F_WRLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
    printf("the wholesaler (pid %d) is done (total desserts: %d)\n",getpid(),sum);
    fflush(stdout);
    lockToWrite.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO,F_SETLKW,&lockToWrite);
}
void signalHandler(int sig){
    terminate = 1;
}
int createSemaphores(sem_t *sem_arr_pusher[],sem_t *sem_arr_chef[]){

    for (int i = 0; i < NUMBEROFINGREDIENTS; i++)
    {
        //sem_unlink(ingredientSemaphoreNames[i]);
        sem_unlink(ingredientSemaphoreNames[i]);
        sem_arr_pusher[i] = sem_open(ingredientSemaphoreNames[i],O_CREAT | O_EXCL, S_IRUSR | S_IWUSR,0);
        if(sem_arr_pusher[i] == SEM_FAILED){
            perror("sem_open ingredient");
            // unlink opened semaphores so far
            for(int j = 0;j<i;j++){
                sem_close(sem_arr_pusher[i]);
                sem_unlink(ingredientSemaphoreNames[i]);
            }
            waitForChildren();
            return -1;
        }

        
    }
    for (int i = 0; i < NUMBER_OF_CHEFS; i++)
    {
        sem_unlink(ingredientSemaphoreNamesForChefs[i]);
        sem_arr_chef[i] = sem_open(ingredientSemaphoreNamesForChefs[i],O_CREAT | O_EXCL, S_IRUSR | S_IWUSR,0);
        if(sem_arr_chef[i] == SEM_FAILED){
            perror("sem_open forchefs");
                        // unlink opened semaphores so far
            for(int j = 0;j<i;j++){
                sem_close(sem_arr_chef[i]);
                sem_unlink(ingredientSemaphoreNamesForChefs[i]);
            }
            waitForChildren();
            return -1;
        }
    }
    return 0;

}
/*
    argv[1]: holds ingredients name for named semaphore
    argv[2]: program name to decide which type of semaphore will be used (named/unnamed)
    argv[3]: shared memory name        
    argv[4]: isIngredient1Index isIngredient2Index isIngredient3Index isIngredient4Index %d%d%d%d, last one will the one which is assigned to 1
    argv[5]: ingredientSemaphoreNamesForChefs for isIngredient1Index
    argv[6]: ingredientSemaphoreNamesForChefs for isIngredient2Index
    argv[7]: ingredientSemaphoreNamesForChefs for isIngredient3Index
*/

/*
const char *ingredientSemaphoreNamesForChefs[]= //{"/MILK2","/FLOUR2","/WALNUTS2","/SUGAR2"};
{"/FS",
"/MS",
"/SW",
"/FM",
"/FW",
"/MW"};
*/
// const char *ingredientSemaphoreNames[]= {"/MILK","/FLOUR","/WALNUTS","/SUGAR"};
int forkPushers(char * sharedMemName,const char *programName){
    for(int i=0;i<NUMBEROFINGREDIENTS ;i++){
        int id = fork();
        if(id == -1){
            perror("Fork Error");
            return -1;
        }
        if(id == 0){
            char ingredientIndexesFormat[]="%d%d%d%d";
            char ingredientIndexes[100];
            char* ingredient_1_semaphore;
            char* ingredient_2_semaphore;
            char* ingredient_3_semaphore;

           //printf("IIII:%d\n",i);
            if(i == 0){
                //MILK
                snprintf(ingredientIndexes,4*sizeof(int),ingredientIndexesFormat,WALNUTS,FLOUR,SUGAR,MILK);
                ingredient_1_semaphore = "/FS";
                ingredient_2_semaphore = "/SW";
                ingredient_3_semaphore = "/FW";
            } else if(i==1){
                //FLOUR
                snprintf(ingredientIndexes,4*sizeof(int),ingredientIndexesFormat,WALNUTS,MILK,SUGAR,FLOUR);
                ingredient_1_semaphore = "/MS";
                ingredient_2_semaphore = "/SW";
                ingredient_3_semaphore = "/MW";
            } else if(i == 2){
                //WALNUTS
                snprintf(ingredientIndexes,4*sizeof(int),ingredientIndexesFormat,FLOUR,MILK,SUGAR,WALNUTS);
                ingredient_1_semaphore = "/MS";
                ingredient_2_semaphore = "/FS";
                ingredient_3_semaphore = "/FM";
            }else { //if(i == 3)
                //SUGAR
                snprintf(ingredientIndexes,4*sizeof(int),ingredientIndexesFormat,FLOUR,MILK,WALNUTS,SUGAR);
                ingredient_1_semaphore = "/MW";
                ingredient_2_semaphore = "/FW";
                ingredient_3_semaphore = "/FM";
            }
            char *const argv[]= {"./pusher",ingredientSemaphoreNames[i],programName,sharedMemName,ingredientIndexes,ingredient_1_semaphore,ingredient_2_semaphore,ingredient_3_semaphore,NULL};
            execv(argv[0],argv);
        } else{
            childProcesses[activeChildProcesses++] = id;
        }

    }
    return 0;
}