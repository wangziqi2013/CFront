
#ifndef _TYPE_H
#define _TYPE_H

#include "hashtable.h"
#include "token.h"

#define SCOPE_LEVEL_GLOBAL 0

enum {
 SCOPE_ENUM   = 0,
 SCOPE_VAR    = 1,
 SCOPE_STRUCT = 2,
 SCOPE_UNION  = 3,
 SCOPE_UDEF   = 4,
 SCOPE_TYPE_COUNT,
};

// A statement block creates a new scope. The bottomost scope is the global scope
typedef struct {
  int level;                            // 0 means global
  hashtable_t *names[SCOPE_TYPE_COUNT]; // enum, var, struct, union, udef
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

scope_t *scope_init(int level);
void scope_free(scope_t *scope);
type_cxt_t *type_init();
void type_free(type_cxt_t *cxt); 
hashtable_t *scope_atlevel(type_cxt_t *cxt, int level, int type);
int scope_numlevel(type_cxt_t *cxt);
void *scope_search(type_cxt_t *cxt, int type, void *name);

#endif