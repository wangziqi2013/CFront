
#ifndef _CGEN_H
#define _CGEN_H

#include "ast.h"
#include "type.h"

typedef {
  type_cxt_t *type_cxt; // We need symbol table in the type context
  
} cgen_cxt_t;

void cgen_global_decl(type_cxt_t *cxt, token_t *global_decl);
void cgen_global_func(type_cxt_t *cxt, token_t *func);
void cgen(type_cxt_t *cxt, token_t *root);

#endif