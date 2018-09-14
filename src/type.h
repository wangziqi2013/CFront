
#ifndef _TYPE_H
#define _TYPE_H

#include "hashtable.h"

typedef struct {
  hashtable_t *enum_const;
} type_cxt_t;

type_cxt_t *type_init();
void type_free(type_cxt_t *cxt); 

#endif