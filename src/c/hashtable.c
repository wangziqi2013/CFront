
#include "hashtable.h"

hashtable_t *ht_init(eq_cb_t eq, hash_cb_t hash) {
  hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
  if(ht == NULL) perror(__func__);
  ht->eq = eq;
  ht->hash = hash;
  ht->mask = HT_INIT_MASK;
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

// Returns an existing slot for key, if it already exists, or an empty one
int ht_find_slot(void **keys, void *key, hashval_t mask) {
  hashval_t begin = hash(key) & mask;
  while(keys[begin] && !eq(keys[begin], key)) begin = (begin + 1) & mask;
  return begin;
}

void ht_resize(hashtable_t *ht) {
  assert(ht->size < ht->capacity);
  ht->capacity *= 2;
  ht->mask |= (ht->mask << 1);
  void **new_keys = (void **)malloc(sizeof(void *) * ht->capacity);
  void **new_values = (void **)malloc(sizeof(void *) * ht->capacity);
  if(new_keys == NULL || new_values == NULL) perror(__func__);

}