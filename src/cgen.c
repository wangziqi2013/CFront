
#include "cgen.h"

// Resolves pending references of the external declaration value
void cgen_resolve_extern(type_cxt_t *cxt, value_t *value) {
  (void)cxt; (void)value;
}

// 1. typedef - must have a name
// 2. extern - If there is init list then this is definition, otherwise just declaration
//    2.1 Function objects must not be declared with extern
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
      error_row_col_exit(decl->offset, "Keyword \"register\" is not allowed for outer-most scope\n");
    } else if(DECL_ISAUTO(basetype->decl_prop)) {
      error_row_col_exit(decl->offset, "Keyword \"auto\" is not allowed for outer-most scope\n");
    } else if((DECL_ISEXTERN(basetype->decl_prop) && !init) || (type_is_func(type))) { 
      // Declaration: has extern, no def, or is function type
      if(name->type == T_) // Extern type must have a name to be imported
        error_row_col_exit(decl->offset, "External declaration must have a name\n");
      else if(type_is_func(type) && DECL_ISEXTERN(basetype->decl_prop))
        error_row_col_exit(decl->offset, "You don't need \"extern\" to declare functions\n");
      else if(type_is_func(type) && init)
        error_row_col_exit(decl->offset, "Function prototype does not allow initialization\n");

      value_t *value = value_init(cxt);
      value->pending = 1;            // If sees pending = 1 we just use an abstracted name for the value
      value->pending_list = list_init();
      if(type_is_array(type) || type_is_func(type)) value->addrtype = ADDR_LABEL;
      else value->addrtype = ADDR_GLOBAL;  // Otherwise it must be a global variable (func are in the next branch)
      value->import_id = cxt->global_import_id++;
      value->type = type;
      list_insert(cxt->import_list, name->str, value);
      ht_insert(cxt->import_index, name->str, value);
      // Also insert into the scope
      if(scope_search(cxt, SCOPE_VALUE, name->str)) 
        error_row_col_exit(decl->offset, "Duplicated global declaration of name \"%s\"\n", name->str);
      scope_top_insert(cxt, SCOPE_VALUE, name->str, value);
    } else { // Defines a new global variable or array - may not have name
      assert(!type_is_func(type));
      assert(type->size != 0UL);
      if(type->size == TYPE_UNKNOWN_SIZE) 
        error_row_col_exit(decl->offset, "Incomplete type for global definition\n");
      if(name->type != T_) { // Named global variable or array
        value_t *value = value_init(cxt);
        if(type_is_array(type)) value->addrtype = ADDR_LABEL;
        else value->addrtype = ADDR_GLOBAL;  // Otherwise it must be a global variable (func are in the next branch)
        value->type = type;
        value->offset = type_alloc_global_data(cxt, type->size);
        // Check whether there is already an declaration or func prototype
        value_t *prev_value = (value_t *)scope_search(cxt, SCOPE_VALUE, name->str);
        if(prev_value) {
          if(prev_value->pending == 0) // Not a declaration - duplicated definition
            error_row_col_exit(decl->offset, "Duplicated global definition of name \"%s\"\n", name->str);
          // Resolve all pending references, and then remove the old entry from global scope
          cgen_resolve_extern(cxt, prev_value);
          void *ret = scope_top_remove(cxt, SCOPE_VALUE, name->str);
          assert(ret); (void)ret;
        }
        void *ret = scope_top_insert(cxt, SCOPE_VALUE, name->str, value);
        assert(!ret); (void)ret;
        if(!DECL_ISSTATIC(basetype->decl_prop)) { // Only export when it is non-globally static
          list_insert(cxt->export_list, name->str, value);
        }
      } else { // Otherwise we only allow comp type or enum with no name
        if(!type_is_comp(type) && !type_is_enum(type)) 
          error_row_col_exit(decl->offset, "Global definition must have a name\n");
      }
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