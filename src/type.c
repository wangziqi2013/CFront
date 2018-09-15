
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
  // TODO: TYPE HASH & COMPARISON
  // cxt->types = ht_init(..., ...);
  return cxt;
}

void type_free(type_cxt_t *cxt) {
  stack_free(cxt->scopes);
  ht_free(cxt->types);
  free(cxt);
}

// Make a copy of the type AST in standard format
token_t *clone_type_ast(token_t *basetype, token_t *decl, int bflen) {
  return NULL;
}