void List_Init(list_t *L) {
 L->head = NULL;
 pthread_mutex_init(&L->lock, NULL);
 }

 void List_Insert(list_t *L, int key) {
 // synchronization not needed
 node_t *new = malloc(sizeof(node_t));
 if (new == NULL) {
 perror("malloc");
 return;
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

 void Hash_Init(hash_t *H) {
 int i;
 for (i = 0; i < BUCKETS; i++)
 List_Init(&H->lists[i]);
 }

 int Hash_Insert(hash_t *H, int key) {
 return List_Insert(&H->lists[key % BUCKETS], key);
 }

 int Hash_Lookup(hash_t *H, int key) {
 return List_Lookup(&H->lists[key % BUCKETS], key);
 }
