
#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

// Must be a power of two
#define HT_INIT_CAPACITY 128
#define HT_INIT_MASK 0x7F
#define HT_RESIZE_THRESHOLD(capacity) (capacity / 8 * 7)
#define HT_NOTFOUND ((void *)-1)
#define HT_REMOVED  ((void *)-2)

#define HT_OP_INSERT 0
#define HT_OP_FIND   1

typedef unsigned long hashval_t;
typedef int (*eq_cb_t)(void *, void *);    // Equality comparison function
typedef int (*cmp_cb_t)(void *, void *);   // Comparison comparison function
typedef hashval_t (*hash_cb_t)(void *);    // Hash value function

typedef struct {
  eq_cb_t eq;
  hash_cb_t hash;
  hashval_t mask;
  int size;
  int capacity;
  void **keys;
  void **values;
} hashtable_t;

int streq_cb(void *a, void *b);
int strcmp_cb(void *a, void *b);
hashval_t strhash_cb(void *a);
hashtable_t *ht_init(eq_cb_t eq, hash_cb_t hash);
hashtable_t *ht_str_init();
void ht_free(hashtable_t *ht);
int ht_size(hashtable_t *ht);
int ht_find_slot(hashtable_t *ht, void **keys, void *key);
void ht_resize(hashtable_t *ht);
void *ht_find(hashtable_t *ht, void *key);
void *ht_insert(hashtable_t *ht, void *key, void *value);

#endif