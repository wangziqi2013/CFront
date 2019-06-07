
#include "cgen.h"

// 1. typedef - must have a name
// 2. extern - If there is init list then this is definition, otherwise just declaration
// 3. auto, register are disallowed
// 4. static means the var is not exposed to other compilation units
// 5. If none storage class then by default it is definition even without init list
void cgen_global_decl(type_cxt_t *cxt, token_t *global_decl) {
  assert(global_decl->type == T_GLOBAL_DECL_ENTRY);
  token_t *basetype = ast_getchild(global_decl, 0);
  token_t *global_var = ast_getchild(global_decl, 1);
  while(global_var) {
    assert(global_var && global_var->type == T_GLOBAL_DECL_VAR);
    token_t *decl = ast_getchild(global_var, 0);
    // Initializer, optional, can be init list of expression
    token_t *init = ast_getchild(global_var, 1); 
    assert(decl && decl->type == T_DECL);
    token_t *exp = ast_getchild(decl, 1);
    token_t *name = ast_getchild(decl, 2); // This could be T_ if it is struct/union/enum
    assert(exp && name);
    // Global var could have storage class but could not be void without derivation
    type_t *type = type_gettype(cxt, decl, basetype, TYPE_ALLOW_STGCLS); 
    
    if(DECL_ISTYPEDEF(basetype->decl_prop)) { // Typedef of a new type
      if(type->size == TYPE_UNKNOWN_SIZE) 
        error_row_col_exit(decl->offset, "Incomplete type in typedef\n");
      else if(name->type == T_) error_row_col_exit(decl->offset, "Typedef'ed type must have a name");
      scope_top_insert(cxt, SCOPE_UDEF, name->str, type);
    } else if(DECL_ISREGISTER(basetype->decl_prop)) {
      error_row_col_exit(decl->type, "Keyword \"register\" is not allowed for outer-most scope\n");
    } else if(DECL_ISAUTO(basetype->decl_prop)) {
      error_row_col_exit(decl->type, "Keyword \"auto\" is not allowed for outer-most scope\n");
    } else if(DECL_ISEXTERN(basetype->decl_prop) && !init) {
      // TODO: INTRODUCING AN EXTERNALLY DEFINED VAR
    } else if(DECL_ISSTATIC(basetype->decl_prop)) {
      // TODO: DO NOT EXPOSE THIS TO EXTERNAL WORLD
    }
    global_var = global_var->sibling; // Process the next global var
  }
}

void cgen_global_func(type_cxt_t *cxt, token_t *func) {
  (void)cxt; (void)root;
}

// Main entry point to code generation
void cgen(type_cxt_t *cxt, token_t *root) {
  assert(root->type == T_ROOT);
  token_t *t = ast_getchild(root, 0);
  while(t) {
    if(t->type == T_GLOBAL_DECL_ENTRY) {
      cgen_global_decl(cxt, t);
    } else if(t->type == T_GLOBAL_FUNC) {
      cgen_global_func(cxt, t);
    } else {
      assert(0);   // Should not appear at global level
    }
    t = t->sibling // Gets NULL if reaches the end
  }
  return;
}