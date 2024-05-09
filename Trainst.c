#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>

sem_t sem_binary;
sem_t sem_waiting;
sem_t sem_passed;
sem_t sem_passing;
int waiting_train;
int passed_train;
int passing_train;


typedef struct args{
    int id_train;
    int time_sleep;
}args;

void exit_with_error(const char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int msleep(long msec){
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int RandRange(int Min, int Max){
    int diff = Max-Min;
    return (int) (((double)(diff+1)/RAND_MAX) * rand() + Min);
}


void* threadFunction(struct args * args) {
    
    //lock mutex
   if(sem_wait(&sem_waiting)==-1)exit_with_error("sem_wait di sem_waiting");
    waiting_train ++;
    printf("Treni che hanno attraversato: %d,Treni che stanno attraversando: %d, Treni che sono in attesa: %d\n", passed_train, passing_train, waiting_train);
    fflush(stdout);
    // Unlock mutex
    if(sem_post(&sem_waiting)==-1)exit_with_error("sem_post di sem_waiting");



    // Wait for sem_binary
    if(sem_wait(&sem_binary) == -1)exit_with_error("sem_wait sem_binary");

    //lock mutex
   if(sem_wait(&sem_waiting) == -1)exit_with_error("sem_wait di sem_waiting 1");
    waiting_train --;
    // Unlock mutex
    if(sem_post(&sem_waiting)==-1)exit_with_error("sem_wait di sem_waiting 1");

    //lock mutex
    if(sem_wait(&sem_passing) ==-1)exit_with_error("sem_wait di sem_passing");
    passing_train++;
    // Unlock mutex
    if(sem_post(&sem_passing) == -1)exit_with_error("sem_wait di sem_passing");

    printf("Treni che hanno attraversato: %d,Treni che stanno attraversando: %d, Treni che sono in attesa: %d\n", passed_train, passing_train, waiting_train);
    fflush(stdout);

    msleep(args->time_sleep);

    //lock mutex
    if(sem_wait(&sem_passing) == -1)exit_with_error("sem_wait di sem_passing 1");  
    passing_train--;
    // Unlock mutex
    if(sem_post(&sem_passing) == -1)exit_with_error("sem_post di sem_passing 1");
    
    // Signal sem_binary
    if(sem_post(&sem_binary) == -1)exit_with_error("sem_post di sem_binary");

    
    // Lock mutex
    if(sem_wait(&sem_passed) == -1) exit_with_error("sem_wait di sem_passed");
    passed_train++;
    printf("Treni che hanno attraversato: %d,Treni che stanno attraversando: %d, Treni che sono in attesa: %d\n", passed_train, passing_train, waiting_train);
    fflush(stdout);
    // Unlock mutex
    if(sem_post(&sem_passed) == -1) exit_with_error("sem_post di sem_passed");

    free(args);


    return NULL;
}

int main(int argc, char* argv[]) {

    int n = atoi(argv[1]);
    //int* binary = malloc(sizeof(int)*n);
  
    waiting_train = 0;
    passed_train = 0;

    pthread_t thread;
    
    int id_train  = -1;

    // Initialize semaphores
    if(sem_init(&sem_binary, 1, n) ==-1)exit_with_error("sem_init di sem_binary");

     // Initialize mutex
    if(sem_init(&sem_waiting, 1, 1) == -1)exit_with_error("sem_init di sem_waiting");

    // Initialize mutex
    if(sem_init(&sem_passed, 1, 1) == -1)exit_with_error("sem_init di sem_passed");

     // Initialize mutex
    if(sem_init(&sem_passing, 1, 1) == -1)exit_with_error("sem_init di sem_passing");

    srand(time(NULL));

    while(1){
        int r = RandRange(atoi(argv[3]), atoi(argv[4]));
        msleep(r);

        struct args * args = malloc (sizeof(struct args));
        id_train ++;
        args->id_train = id_train;
        args->time_sleep = atoi(argv[2]);

        pthread_create(&thread, NULL, (void*)threadFunction, (void *)args);

    }

    // Destroy semaphores
    if(sem_destroy(&sem_binary) == -1)exit_with_error("sem_destroy di sem_binary");

    // Destroy mutex
    if(sem_destroy(&sem_passed) == -1)exit_with_error("sem_destroy di sem_passed");

    // Destroy mutex
    if(sem_destroy(&sem_waiting) == -1)exit_with_error("sem_destroy di sem_waiting");

    // Destroy mutex
    if(sem_destroy(&sem_passing) == -1)exit_with_error("sem_destroy di sem_passing");

    return 0;
}