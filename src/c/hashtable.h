
#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Must be a power of two
#define HT_INIT_CAPACITY 128
#define HT_INIT_MASK 0x7F

typedef unsigned long hashval_t;
typedef int (*eq_cb_t)(void *, void *);
typedef hashval_t (*hash_cb_t)(void *);

typedef struct {
  eq_cb_t eq;
  hash_cb_t hash;
  hashval_t mask;
  int size;
  int capacity;
  void **keys;
  void **values;
} hashtable_t;

hashtable_t *ht_init(eq_cb_t eq, hash_cb_t hash);
void ht_free(hashtable_t *ht);
int ht_find_slot(void **keys, void *key, hashval_t mask);

#endif