
#ifndef _CGEN_H
#define _CGEN_H

#include "ast.h"
#include "type.h"

#define CGEN_GDATA_PADDING 8 // To avoid allocating a zero byte object on the heap

#define CGEN_ARRAY_DEF      0
#define CGEN_ARRAY_DECL     1

typedef struct {
  type_cxt_t *type_cxt;  // Owns memory; will automatically init and free
  list_t *import_list;       // Externally declared variable, function or array - only valid import is pending is 1
  list_t *export_list;  // Non-statically declared global variable, function or array
  list_t *gdata_list;   // A list of global data, i.e. actual storage
  int64_t gdata_offset; // Next global data offset
} cgen_cxt_t;

// Global data container
typedef struct cgen_data_struct_t {
  void *data;      // Actual data; NULL means uninitialized
  type_t *type;    // Type of the global data, which also contains the size
  int offset;      // Offset relative to the beginning of data segment
} cgen_gdata_t;

void cgen_print_cxt(cgen_cxt_t *cxt);

cgen_cxt_t *cgen_init();
void cgen_free(cgen_cxt_t *cxt);

cgen_gdata_t *cgen_gdata_init(cgen_cxt_t *cxt, type_t *type);
void cgen_gdata_free(cgen_gdata_t *gdata);

void cgen_resolve_extern(cgen_cxt_t *cxt, value_t *value);
cgen_gdata_t *cgen_init_list(cgen_cxt_t *cxt, type_t *type, token_t *init, void *parent_p, int parent_offset);
cgen_data_t *cgen_init_array(cgen_cxt_t *cxt, type_t *type, token_t *token);
int64_t cgen_init_array_(cgen_cxt_t *cxt, type_t *type, token_t *token, cgen_data_t *gdata, int64_t offset);
cgen_gdata_t *cgen_init_value(cgen_cxt_t *cxt, type_t *type, token_t *token);
int64_t cgen_init_value_(cgen_cxt_t *cxt, type_t *type, token_t *token, cgen_data_t *gdata, int64_t offset);

void cgen_resolve_array_size(type_t *decl_type, type_t *def_type, token_t *init, int both_decl);
void cgen_global_decl(cgen_cxt_t *cxt, type_t *type, token_t *basetype, token_t *decl, token_t *init);
void cgen_global_def(cgen_cxt_t *cxt, type_t *type, token_t *basetype, token_t *decl, token_t *init);
void cgen_global_func(cgen_cxt_t *cxt, token_t *func);
void cgen_global(cgen_cxt_t *cxt, token_t *global_decl);
void cgen(cgen_cxt_t *cxt, token_t *root);

#endif