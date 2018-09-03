
#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HT_INIT_CAPACITY 128
typedef int (*cb_t)(void *, void *);

typedef struct {
  cb_t eq;
  int size;
  int capacity;
  void **keys;
  void **values;
} hashtable_t;

#endif