
#include "hashtable.h"

hashtable_t *ht_init(cb_t eq) {
  hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
  if(ht == NULL) perror(__func__);
  ht->eq = eq;
  ht->size = 0;
  ht->capacity = HT_INIT_CAPACITY;
  ht->keys = (void **)malloc(sizeof(void *) * HT_INIT_CAPACITY);
  ht->values = (void **)malloc(sizeof(void *) * HT_INIT_CAPACITY);
  if(ht->keys == NULL || ht->values == NULL) perror(__func__);
  memset(ht->keys, 0x00, sizeof(void *) * ht->capacity);
  return ht;
}

void ht_free(hashtable_t *ht) {
  free(ht->keys);
  free(ht->values);
  free(ht);
  return;
}