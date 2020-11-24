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
    count++;
    // just lock critical section
    pthread_mutex_lock(&L->lock);
    new->next = L->head;
    L->head = new;
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

static volatile int bigcount;
pthread_mutex_t lock;

void *unique_Words(char *file_name,hash_t *H)
{
    char *word;
    FILE *file;
    file = fopen(file_name,"r");
    if(file==NULL)
    {
        perror(file_name);
        exit(2);
    }
    while (fscanf(file, "%ms", &word) != EOF)
    {
        pthread_mutex_lock(&lock);
        if(Hash_Lookup(H,word)){
            Hash_Insert(H,word);
            bigcount++;

        }
        pthread_mutex_unlock(&lock);
    }
    // Close file
    fclose(file);
}




int main(int argc, char **argv) {
    hash_t *hash1 = malloc(sizeof(hash_t));
    Hash_Init(hash1);
    int size = argc;
    pthread_t tid[argc];
    pthread_mutex_init(&lock,NULL);

    for(int i = 1; i<size-1; i++)
{
       pthread_create(&tid[i],NULL,unique_Words,unique_Words(argv[i],hash1));


}

    printf("%d \n",count);
}


