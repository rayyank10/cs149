#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <pthread.h>
#include <sys/time.h>


typedef struct __node_t {
    char key;
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
#define MAX_WORDS   1000

int main(int argc, char **argv) {

    hash_t *hash1 = malloc(sizeof(hash_t));
    Hash_Init(hash1);

    FILE *fptr;

    char path[100];
    int i, len, index, isUnique;
    // List of distinct words
    char words[MAX_WORDS][50];
    char word[50];

    // Count of distinct words
    int  count[MAX_WORDS];


    /* Input file path */
    printf("Enter file path: ");
    scanf("%s", path);


    /* Try to open file */
    fptr = fopen(path, "r");

    /* Exit if file not opened successfully */
    if (fptr == NULL)
    {
        printf("Unable to open file.\n");

        exit(EXIT_FAILURE);
    }

    // Initialize words count to 0
    for (i=0; i<MAX_WORDS; i++)
        count[i] = 0;




    index = 0;
    int bigcount;
    while (fscanf(fptr, "%s", word) != EOF)
    {

        // Check if word exits in list of all distinct words
        isUnique = 1;
        for (i=0; i<index && isUnique; i++)
        {
            if (Hash_Lookup(hash1,word))
                isUnique = 0;
        }

        // If word is unique then add it to distinct words list
        // and increment index. Otherwise increment occurrence
        // count of current word.
        if (isUnique)
        {
            Hash_Insert(hash1,word);
            strcpy(words[index], word);
            count[index]++;
            bigcount ++;
            index++;
        }
      else
        {
            count[i - 1]++;
        }
    }

    // Close file
    fclose(fptr);

        printf("%d",bigcount);


    return 0;

}
