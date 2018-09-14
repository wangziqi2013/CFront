
#include "hashtable.h"
#include "error.h"

int streq_cb(void *a, void *b) { return strcmp(a, b) == 0; }
// Credits: K&R C Second Edition Page 144
hashval_t strhash_cb(void *a) { 
  char *s = (char *)a;
  hashval_t hashval;
  for(hashval = (hashval_t)0; *s != '\0'; s++) hashval = *s + 31 * hashval;
  return hashval;
}

hashtable_t *ht_init(eq_cb_t eq, hash_cb_t hash) {
  hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
  if(ht == NULL) syserror(__func__);
  ht->eq = eq;
  ht->hash = hash;
  ht->mask = HT_INIT_MASK;
  ht->size = 0;
  ht->capacity = HT_INIT_CAPACITY;
  ht->keys = (void **)malloc(sizeof(void *) * HT_INIT_CAPACITY);
  ht->values = (void **)malloc(sizeof(void *) * HT_INIT_CAPACITY);
  if(ht->keys == NULL || ht->values == NULL) syserror(__func__);
  memset(ht->keys, 0x00, sizeof(void *) * ht->capacity);
  return ht;
}

hashtable_t *ht_str_init() {
  return ht_init(streq_cb, strhash_cb);
}

void ht_free(hashtable_t *ht) {
  free(ht->keys);
  free(ht->values);
  free(ht);
  return;
}

// Returns an existing slot for key, if it already exists, or an empty one
int ht_find_slot(hashtable_t *ht, void **keys, void *key) {
  hashval_t begin = ht->hash(key) & ht->mask;
  while(keys[begin] && !ht->eq(keys[begin], key)) begin = (begin + 1) & ht->mask;
  return begin;
}

void ht_resize(hashtable_t *ht) {
  assert(ht->size < ht->capacity);
  ht->capacity *= 2;
  ht->mask |= (ht->mask << 1);
  void **new_keys = (void **)malloc(sizeof(void *) * ht->capacity);
  void **new_values = (void **)malloc(sizeof(void *) * ht->capacity);
  if(new_keys == NULL || new_values == NULL) syserror(__func__);
  memset(new_keys, 0x00, sizeof(void *) * ht->capacity);
  for(int i = 0;i < ht->capacity / 2;i++) {
    if(ht->keys[i]) {
      int slot = ht_find_slot(ht, new_keys, ht->keys[i]);
      assert(new_keys[slot] == NULL);
      new_keys[slot] = ht->keys[i];
      new_values[slot] = ht->values[i];
    }
  }
  free(ht->keys);
  free(ht->values);
  ht->keys = new_keys;
  ht->values = new_values;
  return;
}

// Returns value, or HT_NOTFOUND if not found
void *ht_find(hashtable_t *ht, void *key) {
  assert(key != NULL);
  int slot = ht_find_slot(ht, ht->keys, key);
  return ht->keys[slot] == NULL ? HT_NOTFOUND : ht->values[slot];
}

// Returns the value just inserted; return current value otherwise
void *ht_insert(hashtable_t *ht, void *key, void *value) {
  assert(key != NULL);
  if(HT_RESIZE_THRESHOLD(ht->capacity) == ht->size) ht_resize(ht);
  int slot = ht_find_slot(ht, ht->keys, key);
  if(ht->keys[slot]) return ht->values[slot];
  ht->keys[slot] = key;
  ht->values[slot] = value;
  ht->size++;
  return value;
}

// Removes the key and return if it exists; return HT_NOTFOUND otherwise
void *ht_remove(hastable_t *ht, void *key) {
  assert(key != NULL);
  int slot = ht_find_slot(ht, ht->keys, key), curr = slot;
  void *ret = HT_NOTFOUND;
  if(ht->keys[slot]) {
    ret = ht->keys[slot];
    while(keys[(curr + 1) & ht->mask]) curr = (curr + 1) & ht->mask; // Find the last valid element in the current streak
    ht->keys[slot] = ht->keys[curr];
    ht->values[slot] = ht->values[curr];
    ht->size--;
  }
  return ret;
}

int ht_size(hashtable_t *ht) { return ht->size; }