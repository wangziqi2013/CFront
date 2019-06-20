
#include "eval.h"
#include "cgen.h"

const char *cgen_reloc_name[] = {
  "CODE", "DATA",
};

// TODO: ADDING ENUM AS A TYPE
void cgen_typed_print(type_t *type, void *data) {
  if(type_is_char(type)) {
    printf("CHAR \'%s\'", eval_hex_char(*(char *)data));
  } else if(type_is_int(type)) {
    printf("HEX 0x%lX (DEC %ld)\n", *(uint64_t *)data, *(int64_t *)data);
  } else if(type_is_ptr(type)) {
    printf("PTR 0x%016lX\n", *(uint64_t *)data);
  } else if(type_is_comp(type)) {
    if(type_is_struct(type)) printf("STRUCT {");
    else if(type_is_union(type)) printf("UNION {");
    comp_t *comp = type->comp;
    listnode_t *node = list_head(comp->field_list);
    uint8_t *ptr;
    while(node) {
      field_t *field = list_value(node);
      ptr = (uint8_t *)data + field->offset;
      if(field->bitfield_size != -1) {
        assert(type_is_int(field->type));
        uint64_t bf = *(uint64_t *)ptr;
        bf >>= field->bitfield_offset;
        bf &= ((0x1UL << field->bitfield_size) - 1);
        printf("BITFIELD %lX", bf);
      } else {
        cgen_typed_print(field->type, ptr);
      }
      printf(", ");
      node = list_next(node);
    }
    printf("}");
  } else if(type_is_array(type) && type_is_char(type->next)) {
    printf("STR ");
    char *ptr = (char *)data;
    for(int i = 0;i < type->array_size;i++) {
      printf("\'%s\' ", eval_hex_char(*ptr++))
    }
  } 
  else if(type_is_array(type)) {
    printf("ARRAY {");
    uint8_t *ptr = (uint8_t *)data;

    
  }
}

// This function dumps all context info to stdout
void cgen_print_cxt(cgen_cxt_t *cxt) {
  listnode_t *node;
  printf("Context Object @ 0x%016lX\n", (uint64_t)cxt);
  printf("===================================\n");
  printf("Import List\n");
  printf("-----------\n");
  node = list_head(cxt->import_list);
  int count = 0;
  while(node) {
    char *name = (char *)list_key(node);
    value_t *value = (value_t *)list_value(node);
    if(value->pending) { // Do not print non-pending values (i.e. they have been resolved)
      printf("%s\n", type_print_str(0, value->type, name, 0));
      count++;
    }
    node = list_next(node);
  }
  if(count == 0) printf("(Empty)\n");
  putchar('\n');
  printf("Export List\n");
  printf("-----------\n");
  node = list_head(cxt->export_list);
  if(!node) printf("(Empty)\n");
  while(node) {
    char *name = (char *)list_key(node);
    value_t *value = (value_t *)list_value(node);
    printf("%s\n", type_print_str(0, value->type, name, 0));
    node = list_next(node);
  }
  putchar('\n');
  printf("Reloc List\n");
  printf("----------\n");
  node = list_head(cxt->reloc_list);
  if(!node) printf("(Empty)\n");
  while(node) {
    cgen_reloc_t *reloc = (cgen_reloc_t *)list_value(node);
    printf("%s --> %s @ %ld\n", cgen_reloc_name[reloc->from], cgen_reloc_name[reloc->to], reloc->offset);
    node = list_next(node);
  }
}

cgen_cxt_t *cgen_init() {
  cgen_cxt_t *cxt = (cgen_cxt_t *)malloc(sizeof(cgen_cxt_t));
  SYSEXPECT(cxt != NULL);
  memset(cxt, 0x00, sizeof(cgen_cxt_t));
  cxt->type_cxt = type_sys_init();
  cxt->import_list = list_init();
  cxt->export_list = list_init();
  cxt->gdata_list = list_init();
  cxt->gdata_offset = 0L;
  cxt->reloc_list = list_init();
  return cxt;
}

void cgen_free(cgen_cxt_t *cxt) { 
  type_sys_free(cxt->type_cxt);
  list_free(cxt->import_list);
  list_free(cxt->export_list);
  // Free all nodes in the global data list
  listnode_t *node = list_head(cxt->gdata_list);
  while(node) {
    cgen_gdata_free((cgen_gdata_t *)list_value(node));
    node = list_next(node);
  }
  list_free(cxt->gdata_list);
  // Free all nodes in the reloc list
  node = list_head(cxt->reloc_list);
  while(node) {
    cgen_reloc_free((cgen_reloc_t *)list_value(node));
    node = list_next(node);
  }
  list_free(cxt->reloc_list);
  free(cxt); 
}

// This function initializes all fields in the gdata object
// This function also inserts the gdata object into the global gdata list
cgen_gdata_t *cgen_gdata_init(cgen_cxt_t *cxt, type_t *type) {
  cgen_gdata_t *gdata = (cgen_gdata_t *)malloc(sizeof(cgen_gdata_t));
  SYSEXPECT(gdata != NULL);
  memset(gdata, 0x00, sizeof(cgen_gdata_t));
  gdata->type = type;
  gdata->data = (uint8_t *)malloc(type->size + CGEN_GDATA_PADDING); // Avoid zero byte malloc call
  gdata->offset = cxt->gdata_offset; // Assign an offset relative to the data segment
  cxt->gdata_offset += type->size;
  SYSEXPECT(gdata->data);
  list_insert(cxt->gdata_list, NULL, gdata);
  return gdata;
}

void cgen_gdata_free(cgen_gdata_t *gdata) { 
  free(gdata->data);
  free(gdata); 
}

cgen_reloc_t *cgen_reloc_init(cgen_cxt_t *cxt) {
  cgen_reloc_t *reloc = (cgen_reloc_t *)malloc(sizeof(cgen_reloc_t));
  SYSEXPECT(reloc != NULL);
  memset(reloc, 0x00, sizeof(cgen_reloc_t));
  list_insert(cxt->reloc_list, NULL, reloc);
  return reloc;
}
void cgen_reloc_free(cgen_reloc_t *reloc) { free(reloc); }

cgen_gdata_t *cgen_init_comp(cgen_cxt_t *cxt, type_t *type, token_t *token) {
  cgen_gdata_t *gdata = cgen_gdata_init(cxt, type);
  cgen_init_comp_(cxt, type, token, gdata, 0L);
  return gdata;
}

// Accept next write position, returns the next write position after filling current level
int64_t cgen_init_comp_(cgen_cxt_t *cxt, type_t *type, token_t *token, cgen_gdata_t *gdata, int64_t offset) {
  assert(type_is_comp(type) && token->type == T_INIT_LIST);
  assert(ast_child_count(token) == list_size(type->comp->field_list));
  listnode_t *curr_field_node = list_head(type->comp->field_list);
  token_t *curr_elem = ast_getchild(token, 0);
  if(type_is_struct(type)) {
    if(ast_child_count(token) != list_size(type->comp->field_list)) // Check length of the struct init list
      error_row_col_exit(token->offset, "Initializer list for struct must assign a value for every field\n");
    while(curr_elem) {
      assert(curr_field_node);
      field_t *curr_field = (field_t *)list_value(curr_field_node);
      type_t *curr_type = curr_field->type;
      if(type_is_array(curr_type)) {
        offset = cgen_init_array_(cxt, curr_type, curr_elem, gdata, offset);
      } else if(type_is_comp(curr_type)) {
        offset = cgen_init_comp_(cxt, curr_type, curr_elem, gdata, offset);
      } else if(/* is bit field */0) { // TODO: ADD INIT FOR BIT FIELD
      } else {
        offset = cgen_init_value_(cxt, curr_type, curr_elem, gdata, offset);
      }
      curr_elem = curr_elem->sibling;
      curr_field_node = list_next(curr_field_node);
    }
  } else {
    assert(type_is_union(type));
    if(ast_child_count(token) != 1) // Check length of the struct init list
      error_row_col_exit(token->offset, "Initializer list for union must only assing one field\n");
  }
  return offset;
}


cgen_gdata_t *cgen_init_array(cgen_cxt_t *cxt, type_t *type, token_t *token) {
  cgen_gdata_t *gdata = cgen_gdata_init(cxt, type);
  cgen_init_array_(cxt, type, token, gdata, 0L);
  return gdata;
}

// Accept next write position, returns the next write position after filling current level
int64_t cgen_init_array_(cgen_cxt_t *cxt, type_t *type, token_t *token, cgen_gdata_t *gdata, int64_t offset) {
  assert(type_is_array(type) && (token->type == T_INIT_LIST || token->type == T_STR_CONST));
  // This must be true because if there is init list we always know the array size
  assert(type->array_size != -1 && type->size != TYPE_UNKNOWN_SIZE);
  assert(ast_child_count(token) <= type->array_size);
  type_t *elem_type = type->next;
  token_t *curr_elem = ast_getchild(token, 0);
  int count = 0;
  while(curr_elem) {
    if(type_is_array(elem_type)) {
      offset = cgen_init_array_(cxt, elem_type, curr_elem, gdata, offset);
    } else if(type_is_comp(elem_type)) {
      offset = cgen_init_comp_(cxt, elem_type, curr_elem, gdata, offset);
    } else {
      offset = cgen_init_value_(cxt, elem_type, curr_elem, gdata, offset);
    }
    curr_elem = curr_elem->sibling;
    count++;
  }
  assert(count <= type->array_size);
  int remains = type->array_size - count;
  memset(gdata->data + offset, 0x00, remains * elem_type->size); // Fill the remaining space with zero
  return offset + remains * elem_type->size; 
}

// Processes initializer value for global variable
cgen_gdata_t *cgen_init_value(cgen_cxt_t *cxt, type_t *type, token_t *token) {
  cgen_gdata_t *gdata = cgen_gdata_init(cxt, type);
  cgen_init_value_(cxt, type, token, gdata, 0L);
  return gdata;
}

int64_t cgen_init_value_(cgen_cxt_t *cxt, type_t *type, token_t *token, cgen_gdata_t *gdata, int64_t offset) {
  assert(type->size != TYPE_UNKNOWN_SIZE);
  assert(token && token->type != T_INIT_LIST);
  if(token->type != T_STR_CONST) {
    value_t *value = eval_const_to_type(cxt->type_cxt, token, type, TYPE_CAST_IMPLICIT);
    memcpy(gdata->data + offset, value->data, type->size);
  } else {
    assert(token->type == T_STR_CONST);
    str_t *str = eval_const_str_token(token);
    if(type_is_ptr(type) && type_is_char(type->next)) { // Could only initialize char * (optionally qualifiers)
      cgen_gdata_t *gdata_str = cgen_gdata_init(cxt, type_get_strliteral(cxt->type_cxt, str_size(str) + 1, token->offset));
      *(int64_t *)(gdata->data + offset) = gdata_str->offset; // Write relative value of the global data into ptr value
      // Then add a relocation record
      cgen_reloc_t *reloc = cgen_reloc_init(cxt);
      reloc->to = reloc->from = CGEN_RELOC_DATA;
      reloc->offset = gdata->offset + offset; // The address we just wrote relative value of ptr
      reloc->size = type->size; // Pointer size
      assert(type->size == TYPE_PTR_SIZE);
    } else {
      error_row_col_exit(token->offset, "Cannot use string literal to initialize type \"%s\"\n", 
        type_print_str(0, type, NULL, 0));
    }
  }
  return offset + type->size;
}

// Resolves pending references of the external declaration value
void cgen_resolve_extern(cgen_cxt_t *cxt, value_t *value) {
  (void)cxt; (void)value;
}

// This function resolves the array size in case one of the decl or def is incomplete
// 0. If underlying types are non-equivalent or incomplete, report error
// 1. If there is no decl_type (NULL) 
//    1.1 If there is no init list, and def_type has a dimension, then use it
//    1.2 If there is init list, and def_type has a dimension, and init size is larger than this number
//        report error
//    1.3 If there is init list, and def_type has no dimension, then use init size
//    1.4 If none is present, report error
// 2. If decl_type specifies all dimensions
//    2.1 If def_type does not have the first dimension, we set it
//    2.2 If def_type has dimension we compare them, and report error if they are inconsistent
// 3. If decl_type has no dimension
//    3.1 If def_type has dimension we set it
//    3.2 If def_type does not have dimension
//        3.2.1 If init exists use init
//        3.2.2 If init does not exist, report error
// Final result is that we modify decl_type and def_type such that they are consistent in size
// Argument both_decl is set if both of them are declaratins. In this case we do not report error even
// if the size cannot be decided; We implicitly assume that the first two arguments are all valid
void cgen_resolve_array_size(type_t *decl_type, type_t *def_type, token_t *init, int both_decl) {
  assert(def_type && type_is_array(def_type));
  assert(!init || init->type == T_INIT_LIST || init->type == T_STR_CONST);
  assert(!both_decl || (decl_type && def_type && !init));
  if(def_type->next->size == TYPE_UNKNOWN_SIZE) // Always check this for both cases
    error_row_col_exit(def_type->next->offset, "Incomplete array base type\n");

  if(!decl_type) {
    if(def_type->array_size == -1) {
      if(!init) error_row_col_exit(def_type->offset, "Incomplete array type\n"); // Case 1.4
      if(init->type == T_INIT_LIST) {
        def_type->array_size = ast_child_count(init);
        def_type->size = def_type->array_size * def_type->next->size; // Case 1.3
      } else {
        assert(init->type == T_STR_CONST);
        str_t *str = eval_const_str_token(init);
        def_type->array_size = str_size(str) + 1;
        def_type->size = def_type->array_size * def_type->next->size;
        str_free(str);
      }
    } else { // Case 1.1 if no error
      if(init && ast_child_count(init) > def_type->array_size) // Case 1.2
        error_row_col_exit(def_type->offset, "Array initializer list is longer than array type\n");
    }
    return;
  }

  assert(type_is_array(decl_type));
  if(type_cmp(decl_type->next, def_type->next) != TYPE_CMP_EQ) // Case 0
    error_row_col_exit(def_type->offset, "Global array %s has inconsistent base type (%s) with previous declaration (%s)\n",
      both_decl ? "declaration" : "definition",
      type_print_str(0, decl_type, NULL, 0), type_print_str(1, def_type, NULL, 0));
  int decl_size = decl_type->array_size;
  int def_size = def_type->array_size;
  int init_size;
  if(!init) {
    init_size = -1;
  } else if(init->type == T_INIT_LIST) {
    init_size = ast_child_count(init);
  } else {
    assert(init->type == T_STR_CONST);
    str_t *str = eval_const_str_token(init);
    init_size = str_size(str) + 1; // The size of str does not contain terminating zero
    str_free(str);
  }
  int final_size;
  if(decl_size != -1) {
    final_size = decl_size;
    if(def_size != -1) { // Both are valid sizes
      if(def_size != decl_size) 
        error_row_col_exit(def_type->offset, "Global array size inconsistent with declaration\n");
    } else { // Only decl size is valid, set def size, and check init
      def_type->array_size = decl_size;
      def_type->size = decl_size * def_type->next->size;
    }
  } else {
    if(def_size != -1) {
      final_size = def_size;
      decl_type->array_size = def_size;
      decl_type->size = def_size * decl_type->next->size;
    } else if(init_size != -1) {
      final_size = init_size;
      decl_type->array_size = def_type->array_size = init_size;
      decl_type->size = def_type->size = init_size * decl_type->next->size;
    } else {
      // Only report error if one of them is a definition
      if(!both_decl) error_row_col_exit(def_type->offset, "Incomplete global array size\n");
    }
  }
  if(init_size != -1 && init_size > final_size) 
    error_row_col_exit(init->offset, "Array initializer list is longer than array type\n");
  assert(decl_type->array_size == def_type->array_size); // This is even true if both are decl of unknown size
  assert(decl_type->size == def_type->size);
  return;
}

// Processes global declaration, including normal declaration and function prototype
void cgen_global_decl(cgen_cxt_t *cxt, type_t *type, token_t *basetype, token_t *decl, token_t *init) {
  token_t *name = ast_getchild(decl, 2);
  // Declaration: has extern, no def, or is function type
  if(name->type == T_) {// Extern type must have a name to be imported
    error_row_col_exit(decl->offset, "External declaration must have a name\n");
  } else if(type_is_func(type) && DECL_ISEXTERN(basetype->decl_prop)) {
    error_row_col_exit(decl->offset, "You don't need \"extern\" to declare function prototypes\n");
  } else if(type_is_func(type) && init) {
    error_row_col_exit(decl->offset, "Function prototype does not allow initialization\n");
  } else if(type_is_array(type) && type->next->size == TYPE_UNKNOWN_SIZE) {
    error_row_col_exit(decl->offset, "Array declaration using incomplete base type\n");
  }
  
  // Decl after decl or decl after def
  value_t *prev_value = scope_search(cxt->type_cxt, SCOPE_VALUE, name->str);
  if(prev_value) {
    type_t *prev_type = prev_value->type;
    if(type_is_array(type) && type_is_array(prev_type)) { // Array types are compared more carefully
      cgen_resolve_array_size(prev_value->type, type, NULL, CGEN_ARRAY_DECL);
    } else if(type_cmp(type, prev_type) != TYPE_CMP_EQ) {
      error_row_col_exit(decl->offset, "Incompatible global declaration with a previous %s",
        prev_value->pending ? "declaration" : "definition");
    }
  } else {
    value_t *value = value_init(cxt->type_cxt);
    value->pending = 1;            // If sees pending = 1 we just use an abstracted name for the value
    value->pending_list = list_init(); // Destroyed when we resolve the reference
    value->addrtype = ADDR_GLOBAL;
    value->type = type;
    list_insert(cxt->import_list, name->str, value);
    scope_top_insert(cxt->type_cxt, SCOPE_VALUE, name->str, value);
  } 
  return;
}

void cgen_global_def(cgen_cxt_t *cxt, type_t *type, token_t *basetype, token_t *decl, token_t *init) {
  token_t *name = ast_getchild(decl, 2);
  assert(!type_is_func(type) && !type_is_void(type));

  // Unnamed struct, union and enum declaration - do not reserve space
  if(name->type == T_) {
    if(!type_is_comp(type) && !type_is_enum(type)) 
      error_row_col_exit(decl->offset, "Global variable must have a name\n");
    return;
  }
  
  // Check whether there is already an declaration or func prototype
  value_t *value = (value_t *)scope_search(cxt->type_cxt, SCOPE_VALUE, name->str);
  if(value) {
    if(value->pending == 0) // Not a declaration - duplicated definition
      error_row_col_exit(name->offset, "Duplicated global definition of name \"%s\"\n", name->str);
    if(type_is_array(value->type) && type_is_array(type)) // Resolve decl and def array type
      cgen_resolve_array_size(value->type, type, init, CGEN_ARRAY_DEF);
    if(type_cmp(value->type, type) != TYPE_CMP_EQ)
      error_row_col_exit(decl->offset, "Global variable definition has inconsistent type with previous declaration\n");
  } else {
    // Set array size from either type or init
    if(type_is_array(type)) cgen_resolve_array_size(NULL, type, init, CGEN_ARRAY_DEF);
    value = value_init(cxt->type_cxt);
    value->addrtype = ADDR_GLOBAL; 
    value->type = type;
    value->pending = 0;
    scope_top_insert(cxt->type_cxt, SCOPE_VALUE, name->str, value);
  }

  // This is done even if the value is declared previously
  if(!DECL_ISSTATIC(basetype->decl_prop)) list_insert(cxt->export_list, name->str, value);
  if(init) {
    if(type_is_array(type)) {
      cgen_init_array(cxt, type, init);
    } else if(type_is_comp(type)) {
      cgen_init_comp(cxt, type, init);
    } else { // Single variable - eval and do a cast
      cgen_init_value(cxt, type, init);
    }
  }
  // Resolve all pending references, and then remove the old entry from global scope
  if(value->pending) {
    cgen_resolve_extern(cxt, value);
    value->pending = 0;
  }
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
    // Note that we have one type variable for each declaration
    // This type might have unknown size, but we will resolve them later
    // Do not allow void type; function call returning void is fine (will not report error)
    type_t *type = type_gettype(cxt->type_cxt, decl, basetype, 
      TYPE_ALLOW_STGCLS | TYPE_ALLOW_QUAL); 
    
    if(DECL_ISTYPEDEF(basetype->decl_prop)) { // Typedef of a new type
      if(name->type == T_) {
        error_row_col_exit(decl->offset, "Typedef'ed type must have a name");
      }
      scope_top_insert(cxt->type_cxt, SCOPE_UDEF, name->str, type);
    } else if(DECL_ISREGISTER(basetype->decl_prop)) {
      error_row_col_exit(decl->offset, "Keyword \"register\" is not allowed for outer-most scope\n");
    } else if(DECL_ISAUTO(basetype->decl_prop)) {
      error_row_col_exit(decl->offset, "Keyword \"auto\" is not allowed for outer-most scope\n");
    } else if((DECL_ISEXTERN(basetype->decl_prop) && !init) || (type_is_func(type))) { 
      // Declaration or function prototype
      cgen_global_decl(cxt, type, basetype, decl, init);
    } else { // Defines a new global variable or array - may not have name
      cgen_global_def(cxt, type, basetype, decl, init);
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