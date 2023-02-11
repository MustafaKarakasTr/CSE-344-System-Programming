#ifndef __MY_QUEUE__
#define __MY_QUEUE__
#define INITIAL_CAPACITY 10
typedef struct my_queue{
    int capacity;
    int size;
    int * arr;
}my_queue;
int my_queue_init(my_queue * q);
int my_queue_insert(my_queue * q,int data);
int my_queue_remove(my_queue * q,int *removed);
int my_queue_destroy(my_queue * q);
int my_queue_is_empty(my_queue *q);
#endif