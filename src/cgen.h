
#ifndef _CGEN_H
#define _CGEN_H

#include "ast.h"
#include "type.h"

typedef struct {
  type_cxt_t *type_cxt; // We need symbol table in the type context
  list_t *import_list;       // Externally declared variable, function or array
  hashtable_t *import_index; // Index of the above list - we may remove from this list
  list_t *export_list; // Non-statically declared global variable, function or array
  list_t *gdata_list;  // A list of global data, i.e. actual storage
} cgen_cxt_t;

void cgen_global_decl(type_cxt_t *cxt, token_t *global_decl);
void cgen_global_func(type_cxt_t *cxt, token_t *func);
void cgen(type_cxt_t *cxt, token_t *root);

#endif