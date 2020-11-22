#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <pthread.h>
#include <sys/time.h>

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



int main(int argc, char **argv) {
    hash_t *hash1 = malloc(sizeof(hash_t));
    Hash_Init(hash1);

    char *word[50];

    FILE *file;
    file = fopen(argv[1],"r");
    if(file==NULL)
    {
        perror(argv[1]);
        exit(2);
    }
    int bigcount;
    while (fscanf(file, "%s", &word) != EOF)
    {

        if(Hash_Lookup(hash1,&word)){
            Hash_Insert(hash1,&word);
            bigcount++;
        }
        else{
            printf("%s is not a unique word \n",word);
        }



    }



    // Close file
    fclose(file);

    printf("%d",bigcount);

//    Hash_Insert(hash1,"irishmen");
//    if(!Hash_Lookup(hash1,"irishmen"))
//    {
//        printf("here");
//    }
//    else
//    {
//        printf("not here");
//    }


    return 0;









}
