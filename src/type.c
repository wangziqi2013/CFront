
#include "token.h"
#include "type.h"

scope_t *scope_init(int level) {
  scope_t *scope = (scope_t *)malloc(sizeof(scope_t));
  if(scope == NULL) syserror(__func__);
  scope->level = level;
  for(scope_type_t i = 0;i < SCOPE_TYPE_COUNT;i++) scope->names[i] = ht_str_init();
  return scope;
}

void scope_free(scope_t *scope) {
  for(scope_type_t i = 0;i < SCOPE_TYPE_COUNT;i++) ht_free(scope->names[i]);
  free(scope);
  return;
}

type_cxt_t *type_init() {
  type_cxt_t *cxt = (type_cxt_t *)malloc(sizeof(type_cxt_t));
  if(cxt == NULL) syserror(__func__);
  cxt->scopes = stack_init();
  stack_push(scope_init(SCOPE_LEVEL_GLOBAL));
  // TODO: TYPE HASH & COMPARISON
  // cxt->types = ht_init(..., ...);
  return cxt;
}

void type_free(type_cxt_t *cxt) {
  stack_free(cxt->scopes);
  ht_free(cxt->types);
  free(cxt);
}

// Level 0 means global level
scope_t *scope_getlevel(type_cxt_t *cxt, int level) { 
  return (scope_t *)stack_peek_at(cxt->scopes, stack_size(cxt->scopes) - 1 - level); 
}

hashtable_t *name_getlevel(type_cxt_t *cxt, int level, scope_type_t type) {
  return scope_getlevel(cxt, level)->names[type];
}

// Searches all levels of scopes and return the first one; return NULL if not found
void *scope_search(type_cxt_t *cxt, scope_type_t type, void *name) {
  assert(type >=0 && type < SCOPE_TYPE_COUNT && stack_size(cxt->scopes) > 0);
  for(int level = stack_size(cxt->scope) - 1;level >= 0;level--) {
    void *value = ht_find(name_getlevel(cxt, level, type), name);
    if(value != HT_NOTFOUND) return value;
  }
  return NULL;
}

// Make a copy of the type AST in standard format
token_t *clone_type_ast(token_t *basetype, token_t *decl, int bflen) {
  return NULL;
}