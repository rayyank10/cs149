#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct __node_t {
    int key;
    struct __node_t *next;
} node_t;

// basic list structure (one used per list)
typedef struct __list_t {
    node_t *head;
    pthread_mutex_t lock;
} list_t;

void List_Init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

int List_Insert(list_t *L, int key) {
    // synchronization not needed
    node_t *new = malloc(sizeof(node_t));
    if (new == NULL) {
        perror("malloc");
    }
    new->key = key;

    // just lock critical section
    pthread_mutex_lock(&L->lock);
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&L->lock);
}
int List_Lookup(list_t *L, int key) {
    int rv = -1;
    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head;
    while (curr) {
        if (curr->key == key) {
            rv = 0;
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&L->lock);
    return rv; // now both success and failure
}

#define BUCKETS (101)
typedef struct __hash_t {
    list_t lists[BUCKETS];
} hash_t;

//taken from stackoverflow
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
void Hash_Init(hash_t *H) {
    int i;
    for (i = 0; i < BUCKETS; i++)
        List_Init(&H->lists[i]);
}

int Hash_Insert(hash_t *H, char* key) {
    return List_Insert(&H->lists[hash(key) % BUCKETS], key);
}

int Hash_Lookup(hash_t *H, char* key) {
    return List_Lookup(&H->lists[hash(key) % BUCKETS], key);
}

static volatile int counter = 0;
pthread_mutex_t lock;
void *mythread(void *arg)
{
    printf("/s: begin\n",(char *)arg);
    for(int i = 0; i <1e7; i++)
    {
        pthread_mutex_lock(&lock);
        counter = counter + 1;
        pthread_mutex_unlock(&lock);
    }
    printf("%s: done\n",(char *)arg);
}
int main(int argc, char **argv) {
   pthread_t p1,p2;
   pthread_mutex_init(&lock,NULL);
   printf("counter = %d\n",counter);
   pthread_create(&p1,NULL,mythread,"A");
    pthread_create(&p2,NULL,mythread,"B");

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    printf("counter = %d\n", counter);

}
