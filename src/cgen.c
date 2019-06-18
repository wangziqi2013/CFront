
#include "eval.h"
#include "cgen.h"

cgen_cxt_t *cgen_init() {
  cgen_cxt_t *cxt = (cgen_cxt_t *)malloc(sizeof(cgen_cxt_t));
  SYSEXPECT(cxt != NULL);
  memset(cxt, 0x00, sizeof(cgen_cxt_t));
  cxt->type_cxt = type_sys_init();
  cxt->import_list = list_init();
  cxt->import_index = ht_str_init();
  cxt->export_list = list_init();
  cxt->gdata_list = list_init();
  return cxt;
}

void cgen_free(cgen_cxt_t *cxt) { 
  type_sys_free(cxt->type_cxt);
  list_free(cxt->import_list);
  ht_free(cxt->import_index);
  list_free(cxt->export_list);
  // Free all nodes in the global data list
  listnode_t *node = list_head(cxt->gdata_list);
  while(node) {
    cgen_gdata_free((cgen_gdata_t *)list_value(node));
    node = list_next(node);
  }
  list_free(cxt->gdata_list);
  free(cxt); 
}

cgen_gdata_t *cgen_gdata_init() {
  cgen_gdata_t *gdata = (cgen_gdata_t *)malloc(sizeof(cgen_gdata_t));
  SYSEXPECT(gdata != NULL);
  memset(gdata, 0x00, sizeof(cgen_gdata_t));
  return gdata;
}
void cgen_gdata_free(cgen_gdata_t *gdata) { free(gdata); }

// Resolves pending references of the external declaration value
void cgen_resolve_extern(cgen_cxt_t *cxt, value_t *value) {
  (void)cxt; (void)value;
}

// Processes and materializes initialization list, returns a pointer to the next write position
// 1. Top level function should call with parent_p = NULL and parent_offset equals any value
// 2. The size of data is already given by the type
// 3. Return value points to the end of the buffer after processing the current list; To obtain the 
//    head, just subtract the type size from the pointer value
cgen_gdata_t *cgen_init_list(cgen_cxt_t *cxt, type_t *type, token_t *init, void *parent_p, int parent_offset) {
  assert(init->type == T_INIT_LIST);
  if(!type_is_array(type) && !type_is_comp(type))
    error_row_col_exit(init->offset, "Initializer list can only be used to initialize array or composite types\n");
  // If it is array type, and the size of the first dimension is not defined, we fill the value using the 
  // length of the init list
  if(type->size == TYPE_UNKNOWN_SIZE) {
    if(type_is_array(type)) {
     
    } else {
      error_row_col_exit(type->offset, "Could not initialize incomplete composite type\n");
    }
  }
  uint8_t *ret = (uint8_t *)parent_p;
  int offset = parent_offset; (void)offset; // TODO: REMOVE THIS
  if(parent_p == NULL) {
    ret = (uint8_t *)malloc(type->size); 
    SYSEXPECT(ret != NULL);
    memset(ret, 0x00, type->size);
    offset = 0;
  }
  token_t *entry = ast_getchild(init, 0);
  while(entry) {
    
    entry = entry->sibling;
  }
  return (void *)ret;
}

// Processes initializer value for global variable
cgen_gdata_t *cgen_init_value(cgen_cxt_t *cxt, type_t *type, token_t *token) {
  assert(type->size != TYPE_UNKNOWN_SIZE);
  assert(token);
  cgen_gdata_t *gdata = cgen_gdata_init();
  gdata->type = type;
  gdata->data = malloc(type->size + CGEN_GDATA_PADDING); // Avoid zero byte data
  value_t *value = eval_const_to_type(cxt->type_cxt, token, type, TYPE_CAST_IMPLICIT);
  memcpy(gdata->data, value->data, type->size);
  return gdata;
}

// Processes global declaration, including normal declaration and function prototype
void cgen_global_decl(cgen_cxt_t *cxt, type_t *type, token_t *name, token_t *global_var, token_t *basetype) {
  token_t *decl = ast_getchild(global_var, 0);
  token_t *init = ast_getchild(global_var, 1);
  // Declaration: has extern, no def, or is function type
  if(name->type == T_) {// Extern type must have a name to be imported
    error_row_col_exit(decl->offset, "External declaration must have a name\n");
  } else if(type_is_func(type) && DECL_ISEXTERN(basetype->decl_prop)) {
    error_row_col_exit(decl->offset, "You don't need \"extern\" to declare function prototypes\n");
  } else if(type_is_func(type) && init) {
    error_row_col_exit(decl->offset, "Function prototype does not allow initialization\n");
  }
  value_t *value = value_init(cxt->type_cxt);
  value->pending = 1;            // If sees pending = 1 we just use an abstracted name for the value
  value->pending_list = list_init();
  value->addrtype = ADDR_GLOBAL;
  value->type = type;
  list_insert(cxt->import_list, name->str, value);
  ht_insert(cxt->import_index, name->str, value);
  // Also insert into the scope
  if(scope_search(cxt->type_cxt, SCOPE_VALUE, name->str)) 
    error_row_col_exit(decl->offset, "Duplicated global declaration of name \"%s\"\n", name->str);
  scope_top_insert(cxt->type_cxt, SCOPE_VALUE, name->str, value);
  return;
}

// 1. typedef - must have a name
// 2. extern - If there is init list then this is definition, otherwise just declaration
//    2.1 Function objects must not be declared with extern
// 3. auto, register are disallowed
// 4. static means the var is not exposed to other compilation units
// 5. If none storage class then by default it is definition even without init list
void cgen_global(cgen_cxt_t *cxt, token_t *global_decl) {
  assert(global_decl->type == T_GLOBAL_DECL_ENTRY);
  token_t *basetype = ast_getchild(global_decl, 0);
  token_t *global_var = ast_getchild(global_decl, 1);
  while(global_var) {
    assert(global_var && global_var->type == T_GLOBAL_DECL_VAR);
    token_t *decl = ast_getchild(global_var, 0);
    // Initializer, optional, can be T_INIT_LIST or expression node
    token_t *init = ast_getchild(global_var, 1); 
    assert(decl && decl->type == T_DECL);
    token_t *name = ast_getchild(decl, 2); // This could be T_ if it is struct/union/enum
    assert(name);
    // Global var could have storage class but could not be void without derivation
    type_t *type = type_gettype(cxt->type_cxt, decl, basetype, TYPE_ALLOW_STGCLS); 
    
    if(DECL_ISTYPEDEF(basetype->decl_prop)) { // Typedef of a new type
      if(type->size == TYPE_UNKNOWN_SIZE) {
        error_row_col_exit(decl->offset, "Incomplete type in typedef\n");
      } else if(name->type == T_) {
        error_row_col_exit(decl->offset, "Typedef'ed type must have a name");
      }
      scope_top_insert(cxt->type_cxt, SCOPE_UDEF, name->str, type);
    } else if(DECL_ISREGISTER(basetype->decl_prop)) {
      error_row_col_exit(decl->offset, "Keyword \"register\" is not allowed for outer-most scope\n");
    } else if(DECL_ISAUTO(basetype->decl_prop)) {
      error_row_col_exit(decl->offset, "Keyword \"auto\" is not allowed for outer-most scope\n");
    } else if((DECL_ISEXTERN(basetype->decl_prop) && !init) || (type_is_func(type))) { 
      cgen_global_decl(cxt, type, name, global_var, basetype);
    } else { // Defines a new global variable or array - may not have name
      assert(!type_is_func(type) && !type_is_void(type));
      // If the array has an initializer list, we could derive its element count and size
      if(type->size == TYPE_UNKNOWN_SIZE) {
        if(type_is_array(type) && init) {
          assert(type->array_size == -1);
          if(type->next->size == TYPE_UNKNOWN_SIZE) // The element size must be known
            error_row_col_exit(type->offset, "Array initialization with incomplete element type\n");
          int child_count = ast_child_count(init);
          assert(child_count >= 0);
          type->array_size = child_count;
          type->size = type->next->size * child_count;
        } else {
          error_row_col_exit(decl->offset, "Could not define global variable with incomplete type\n");
        }
      }
      if(name->type != T_) { // Named global variable or array
        value_t *value = value_init(cxt->type_cxt);
        value->addrtype = ADDR_GLOBAL; 
        value->type = type;
        // Check whether there is already an declaration or func prototype
        value_t *prev_value = (value_t *)scope_search(cxt->type_cxt, SCOPE_VALUE, name->str);
        if(prev_value) {
          if(prev_value->pending == 0) // Not a declaration - duplicated definition
            error_row_col_exit(decl->offset, "Duplicated global definition of name \"%s\"\n", name->str);
          // TODO: CHECK WHETHER ARRAY RANGE ARE CONSISTENT
          // Resolve all pending references, and then remove the old entry from global scope
          cgen_resolve_extern(cxt, prev_value);
          void *ret = scope_top_remove(cxt->type_cxt, SCOPE_VALUE, name->str);
          assert(ret); (void)ret;
        }
        void *ret = scope_top_insert(cxt->type_cxt, SCOPE_VALUE, name->str, value);
        assert(!ret); (void)ret;
        if(!DECL_ISSTATIC(basetype->decl_prop)) { // Only export when it is non-globally static
          list_insert(cxt->export_list, name->str, value);
        }
        // TODO: PROCESS INIT LIST
      } else if(!type_is_comp(type) && !type_is_enum(type)) { // Otherwise we only allow comp type or enum with no name
        error_row_col_exit(decl->offset, "Global variable must have a name\n");
      }
    }
    global_var = global_var->sibling; // Process the next global var
  }
}

void cgen_global_func(cgen_cxt_t *cxt, token_t *func) {
  (void)cxt; (void)func;
}

// Main entry point to code generation
void cgen(cgen_cxt_t *cxt, token_t *root) {
  assert(root->type == T_ROOT);
  token_t *t = ast_getchild(root, 0);
  while(t) {
    if(t->type == T_GLOBAL_DECL_ENTRY) {
      cgen_global(cxt, t);
    } else if(t->type == T_GLOBAL_FUNC) {
      cgen_global_func(cxt, t);
    } else {
      assert(0);   // Should not appear at global level
    }
    t = t->sibling; // Gets NULL if reaches the end
  }
  return;
}