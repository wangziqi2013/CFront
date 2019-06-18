
#ifndef _CGEN_H
#define _CGEN_H

#include "type.h"
#include "ast.h"

// Global data container
typedef struct {
  void *data;      // Actual data; NULL means uninitialized
  type_t *type;    // Type of the global data, which also contains the size
} cgen_gdata_t;

cgen_gdata_t *cgen_init_gdata();
void cgen_free_gdata(cgen_gdata_t *gdata);

void cgen_global_decl(type_cxt_t *cxt, token_t *global_decl);
void cgen_global_func(type_cxt_t *cxt, token_t *func);
void cgen(type_cxt_t *cxt, token_t *root);

#endif