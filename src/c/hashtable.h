
#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <stdlib.h>

#define HT_INIT_CAPACITY 128

typedef struct {
  int (*eq)(void *, void *);
  int size;
  int capacity;
  void **keys;
  void **values;
} hashtable_t;

#endif