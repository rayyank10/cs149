#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
int count;

typedef struct __node_t {
    char* key;
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

int List_Insert(list_t *L, char* key) {
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
    count++;
    pthread_mutex_unlock(&L->lock);
}
int List_Lookup(list_t *L, char* key) {
    int rv = -1;

    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head;
    while (curr) {
        if (strcmp(curr->key,key)==0) {
            rv = 0;
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&L->lock);
    return rv; // now both success and failure
}

#define BUCKETS (256)
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

struct _args
{
    char *argv;
    struct hash_t *hash ;
};
pthread_mutex_t lock;

void *unique_Words(void *v)
{
    struct _args *args  = v;
    char *word;
    FILE *file;
    file = fopen(args->argv,"r");
    if(file==NULL)
    {
        perror(args->argv);
        exit(2);
    }
    while (fscanf(file, "%ms", &word) != EOF)
    {
        pthread_mutex_lock(&lock);
        if(Hash_Lookup(args->hash,word)){
            Hash_Insert(args->hash,word);

        }
        pthread_mutex_unlock(&lock);
    }
    // Close file
    fclose(file);
}




int main(int argc, char **argv) {
    struct _args arguments;
    hash_t *hash1 = malloc(sizeof(hash_t));
    Hash_Init(hash1);
    arguments.hash = hash1;
    int size = argc;
    pthread_t tid[argc];
    pthread_mutex_init(&lock,NULL);

    for(int i = 1; i<size; i++)
{
        arguments.argv = argv[i];
       pthread_create(&tid[i],NULL,unique_Words,&arguments);


}

    printf("%d \n",count);
}
