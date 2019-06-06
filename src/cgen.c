
#include "cgen.h"

void cgen_global_decl(type_cxt_t *cxt, token_t *decl) {
  (void)cxt; (void)root;
}

void cgen_global_func(type_cxt_t *cxt, token_t *func) {
  (void)cxt; (void)root;
}

// Main entry point to code generation
void cgen(type_cxt_t *cxt, token_t *root) {
  assert(root->type == T_ROOT);
  token_t *t = ast_getchild(root, 0);
  if(t->type == T_GLOBAL_DECL_ENTRY) {
    cgen_global_decl(cxt, t);
  } else if(t->type == T_GLOBAL_FUNC) {
    cgen_global_func(cxt, t);
  } else {
    assert(0); // Should not appear at global level
  }
  return;
}