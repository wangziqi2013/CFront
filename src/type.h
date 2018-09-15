
#ifndef _TYPE_H
#define _TYPE_H

#include "hashtable.h"
#include "token.h"

// A statement block creates a new scope. The bottomost scope is the global scope
typedef struct {
  int level;             // 0 means global
  hashtable_t *enums;    // Maps enum constant to value object
  hashtable_t *vars;     // Variables
  hashtable_t *structs;
  hashtable_t *unions;
  hashtable_t *udefs;
} scope_t;

typedef struct {
  stack_t *scopes;
  hashtable_t *types;    // Maps type id to type object
} type_cxt_t;

typedef unsigned long typeid_t;
typedef struct {
  typeid_t typeid;
} value_t;

typedef struct {
  typeid_t typeid;
  token_t *decl;
  unsigned char size;
} type_t;

type_cxt_t *type_init();
void type_free(type_cxt_t *cxt); 

#endif