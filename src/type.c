
#include "token.h"
#include "type.h"
#include "eval.h"
#include "ast.h"
#include "str.h"

int_prop_t ints[11] = { // Integer sign and size, using index of base type
  {-1, -1}, // BASETYPE_NONE, 0x00
  {1, TYPE_CHAR_SIZE}, // BASETYPE_CHAR, 0x01
  {1, TYPE_SHORT_SIZE}, // BASETYPE_SHORT, 0x02
  {1, TYPE_INT_SIZE}, // BASETYPE_INT, 0x03
  {1, TYPE_LONG_SIZE}, // BASETYPE_LONG, 0x04
  {0, TYPE_CHAR_SIZE}, // BASETYPE_UCHAR, 0x05
  {0, TYPE_SHORT_SIZE}, // BASETYPE_USHORT, 0x06
  {0, TYPE_INT_SIZE}, // BASETYPE_UINT, 0x07
  {0, TYPE_LONG_SIZE}, // BASETYPE_ULONG, 0x08
  {1, TYPE_LLONG_SIZE}, // BASETYPE_LLONG, 0x09
  {0, TYPE_LLONG_SIZE}, // BASETYPE_ULLONG, 0x0A
};

// The following are type templates, they cannot be directly used, and must be obtained using type_init_from
type_t type_builtin_ints[11] = {
  {0, NULL, {NULL}, {0}, NULL, NULL, 0}, // Integer type begins at index 1
  {BASETYPE_CHAR, NULL, {NULL}, {0}, NULL, NULL, TYPE_CHAR_SIZE},
  {BASETYPE_SHORT, NULL, {NULL}, {0}, NULL, NULL, TYPE_SHORT_SIZE},
  {BASETYPE_INT, NULL, {NULL}, {0}, NULL, NULL, TYPE_INT_SIZE},
  {BASETYPE_LONG, NULL, {NULL}, {0}, NULL, NULL, TYPE_LONG_SIZE},
  {BASETYPE_UCHAR, NULL, {NULL}, {0}, NULL, NULL, TYPE_CHAR_SIZE},
  {BASETYPE_USHORT, NULL, {NULL}, {0}, NULL, NULL, TYPE_SHORT_SIZE},
  {BASETYPE_UINT, NULL, {NULL}, {0}, NULL, NULL, TYPE_INT_SIZE},
  {BASETYPE_ULONG, NULL, {NULL}, {0}, NULL, NULL, TYPE_LONG_SIZE},
  {BASETYPE_LLONG, NULL, {NULL}, {0}, NULL, NULL, TYPE_LLONG_SIZE},
  {BASETYPE_ULLONG, NULL, {NULL}, {0}, NULL, NULL, TYPE_LLONG_SIZE},
};

type_t type_builtin_void = {
  BASETYPE_VOID, NULL, {NULL}, {0}, NULL, NULL, TYPE_VOID_SIZE
};
type_t type_builtin_const_char = { // const char type
  BASETYPE_CHAR | DECL_CONST_MASK, NULL, {NULL}, {0}, NULL, NULL, TYPE_CHAR_SIZE
}; 
// String type is evaluated as const char [length]; here is a template
type_t type_builtin_string_template = { // Should change size to actual length when copy
  TYPE_OP_ARRAY_SUB, &type_builtin_const_char, {NULL}, {0}, NULL, NULL, TYPE_UNKNOWN_SIZE 
};

obj_free_func_t obj_free_func_list[OBJ_TYPE_COUNT + 1] = {  // Object free functions
  type_free,
  comp_free,
  field_free,
  enum_free,
  value_free,
  NULL,       // Sentinel - will segment fault
};

// Argument full_size includes the trailing '\0'
type_t *type_get_strliteral(type_cxt_t *cxt, size_t full_size, char *offset) {
  type_t *const_char_type = type_init_from(cxt, &type_builtin_const_char, offset);
  type_t *ret = type_init_from(cxt, &type_builtin_string_template, offset);
  ret->array_size = full_size;
  ret->size = full_size * TYPE_CHAR_SIZE;
  ret->next = const_char_type; 
  return ret;
}

// We provide 4 channels such that they can be used in the same printf function call
// (C semsntics require that arguments be fully evaluated before function call)
char *type_print_str(int channel, type_t *type, const char *name, int print_comp_body) {
  static str_t *channels[TYPE_PRINT_CHANNEL_MAX] = {NULL, }; // All NULL
  assert(channel >= 0 && channel < TYPE_PRINT_CHANNEL_MAX);
  if(channels[channel]) str_free(channels[channel]);
  channels[channel] = str_init();
  type_print(type, name, channels[channel], print_comp_body, 0);
  return str_cstr(channels[channel]);
}

// Prints a type in string on stdout
// type is the type object, name is shown as the inner most operand of the type expression, NULL means no name
// The top level should call this function with s == NULL, name == NULL. The return value contains the type string
str_t *type_print(type_t *type, const char *name, str_t *s, int print_comp_body, int level) {
  assert(type);
  if(!s) s = str_init();
  for(int i = 0;i < level * 2;i++) str_append(s, ' '); // Padding spaces to the level
  // Find base type. Stop at the end or at a udef'ed name
  type_t *basetype = type;
  while(basetype->next && !basetype->udef_name) basetype = basetype->next;

  // First, print base type, qualifier and storage class, if it is a primitive type, with a space at the end
  // typedef is also printed
  str_concat(s, token_decl_print(basetype->decl_prop));
  str_append(s, ' ');

  // This does not include qualifier and storage class
  decl_prop_t base = BASETYPE_GET(basetype->decl_prop);
  assert(base != BASETYPE_UDEF); // Must have already been translated to other types
  
  // If base type is composite, print its body
  if(basetype->udef_name) { // This has highest priority - just print udef name
    str_concat(s, basetype->udef_name);
    str_append(s, ' ');
  } else if(base == BASETYPE_STRUCT || base == BASETYPE_UNION) {
    comp_t *comp = basetype->comp;
    if(comp->name) { str_concat(s, comp->name); str_append(s, ' '); } // Name followed by a space
    if(print_comp_body && comp->has_definition) {
      str_concat(s, "{\n");
      listnode_t *node = list_head(comp->field_list);
      while(node) {
        field_t *field = (field_t *)list_value(node);
        // Note: For self-pointing struct member this will incur infinite loop
        // Check whether it is pointer type, and do not print body if positive
        int print_field_body = !TYPE_OP_GET(field->type->decl_prop); // If derived type do not print body
        str_t *field_s = type_print(field->type, field->name, NULL, print_field_body, level + 1);
        str_concat(s, field_s->s);
        str_free(field_s);
        if(field->bitfield_size != -1) {
          str_concat(s, " : ");
          str_print_int(s, field->bitfield_size);
        }
        str_concat(s, "; @ ");
        str_print_int(s, field->offset); // Append field offset
        if(field->bitfield_size != -1) {
          str_concat(s, " bit ");
          str_print_int(s, field->bitfield_offset);
        }
        str_append(s, '\n');
        node = list_next(node);
      }
      for(int i = 0;i < level * 2;i++) str_append(s, ' ');
      str_concat(s, "} ");
    }
  } else if(base == BASETYPE_ENUM) {
    enum_t *enu = basetype->enu;
    if(enu->name) { str_concat(s, enu->name); str_append(s, ' '); } 
    str_concat(s, "{\n");
    listnode_t *node = list_head(enu->field_list);
    while(node) {
      for(int i = 0;i < level * 2 + 2;i++) str_append(s, ' ');
      str_concat(s, (const char *)list_key(node));
      str_concat(s, " = ");
      str_print_int(s, (int)(long)list_value(node));
      str_concat(s, ",\n");
      node = list_next(node);
    }
    for(int i = 0;i < level * 2;i++) str_append(s, ' ');
    str_concat(s, "} ");
  } else {} // All other types; Nothing to do

  // We next build the type expression
  type_t *prev = NULL;
  str_t *decl_s = str_init();
  if(name) str_concat(decl_s, name);
  while(type->next) { // Means it is not a base type
    decl_prop_t op = TYPE_OP_GET(type->decl_prop);
    assert(op == TYPE_OP_ARRAY_SUB || op == TYPE_OP_DEREF || op == TYPE_OP_FUNC_CALL);
    if(op == TYPE_OP_ARRAY_SUB) {
      if(prev && TYPE_OP_GET(prev->decl_prop) == TYPE_OP_DEREF) {
        str_prepend(decl_s, '(');
        str_append(decl_s, ')');
      }
      str_append(decl_s, '[');
      if(type->array_size != -1) str_print_int(decl_s, type->array_size);
      str_concat(decl_s, "]"); // Always leave a space at the end
    } else if(op == TYPE_OP_DEREF) {
      char *qualifier = token_decl_print(type->decl_prop & DECL_QUAL_MASK); // Only prints qualifier
      if(*qualifier) { // Qualifiers are surrounded by a pair of spaces
        str_prepend(decl_s, ' ');
        str_prepend_str(decl_s, qualifier);
        str_prepend(decl_s, ' '); // Separate qualifiers
      }
      str_prepend(decl_s, '*'); // * followed by qualifiers
    } else if(op == TYPE_OP_FUNC_CALL) {
      if(prev && TYPE_OP_GET(prev->decl_prop) == TYPE_OP_DEREF) {
        str_prepend(decl_s, '(');
        str_append(decl_s, ')');
      }
      str_append(decl_s, '(');
      listnode_t *arg = list_head(type->arg_list);
      while(arg) {
        str_t *arg_s = type_print(list_value(arg), list_key(arg), NULL, 0, 0); // Always do not print body
        str_concat(decl_s, arg_s->s);
        str_free(arg_s);
        if((arg = list_next(arg)) != NULL) str_concat(decl_s, ", "); // If there is more arguments
        else if(type->vararg) str_concat(decl_s, ", ...");
      }
      str_append(decl_s, ')');
    }
    prev = type;
    type = type->next;
  }
  str_concat(s, decl_s->s);
  str_free(decl_s);

  return s;
}

scope_t *scope_init(int level) {
  scope_t *scope = (scope_t *)malloc(sizeof(scope_t));
  SYSEXPECT(scope != NULL);
  scope->level = level;
  for(int i = 0;i < SCOPE_TYPE_COUNT;i++) scope->names[i] = ht_str_init();
  for(int i = 0;i < OBJ_TYPE_COUNT;i++) scope->objs[i] = list_init();
  return scope;
}

void scope_free(scope_t *scope) {
  // Free objects first, then free the list
  for(int i = 0;i < OBJ_TYPE_COUNT;i++) {
    obj_free_func_t func = obj_free_func_list[i];
    listnode_t *curr = list_head(scope->objs[i]);
    while(curr) { func(curr->value); curr = list_next(curr); }
    list_free(scope->objs[i]);
  }
  for(int i = 0;i < SCOPE_TYPE_COUNT;i++) ht_free(scope->names[i]);
  free(scope);
  return;
}

void init_builtin_types() {
  
  return;
}

type_cxt_t *type_sys_init() {
  init_builtin_types();
  type_cxt_t *cxt = (type_cxt_t *)malloc(sizeof(type_cxt_t));
  SYSEXPECT(cxt != NULL);
  cxt->scopes = stack_init();
  scope_recurse(cxt);
  return cxt;
}

void type_sys_free(type_cxt_t *cxt) {
  while(scope_numlevel(cxt)) scope_decurse(cxt); // First pop all scopes
  stack_free(cxt->scopes);
  free(cxt);
}

hashtable_t *scope_atlevel(type_cxt_t *cxt, int level, int domain) {
  return ((scope_t *)stack_peek_at(cxt->scopes, stack_size(cxt->scopes) - 1 - level))->names[domain];
}
hashtable_t *scope_top_name(type_cxt_t *cxt, int domain) { return ((scope_t *)stack_peek_at(cxt->scopes, 0))->names[domain]; }
int scope_numlevel(type_cxt_t *cxt) { return stack_size(cxt->scopes); }
void scope_recurse(type_cxt_t *cxt) { stack_push(cxt->scopes, scope_init(scope_numlevel(cxt))); }
void scope_decurse(type_cxt_t *cxt) { scope_free(stack_pop(cxt->scopes)); }
void scope_top_insert(type_cxt_t *cxt, int domain, void *key, void *value) { 
  ht_insert(scope_top_name(cxt, domain), key, value); 
}

void *scope_top_remove(type_cxt_t *cxt, int domain, void *key) {
  void *ht_ret = ht_remove(scope_top_name(cxt, domain), key);
  return ht_ret == HT_NOTFOUND ? NULL : ht_ret;
}

// Return NULL if the key does not exist in the domain
void *scope_top_find(type_cxt_t *cxt, int domain, void *key) { 
  void *ht_ret = ht_find(scope_top_name(cxt, domain), key); 
  return ht_ret == HT_NOTFOUND ? NULL : ht_ret;
}

// Searches all levels of scopes and return the first one; return NULL if not found
void *scope_search(type_cxt_t *cxt, int domain, void *name) {
  assert(domain >= 0 && domain < SCOPE_TYPE_COUNT && scope_numlevel(cxt) > 0);
  for(int level = scope_numlevel(cxt) - 1;level >= 0;level--) {
    void *value = ht_find(scope_atlevel(cxt, level, domain), name);
    if(value != HT_NOTFOUND) return value;
  }
  return NULL;
}

void scope_top_obj_insert(type_cxt_t *cxt, int domain, void *obj) {
  assert(domain >= 0 && domain < OBJ_TYPE_COUNT);
  list_t *list = (list_t *)((scope_t *)stack_peek_at(cxt->scopes, 0))->objs[domain];
  list_insert(list, NULL, obj);
}

// NOTE: This function DOES NOT initialize arg_list and arg_index, because they may be used for other purposes
type_t *type_init(type_cxt_t *cxt) {
  type_t *type = (type_t *)malloc(sizeof(type_t));
  SYSEXPECT(type != NULL);
  memset(type, 0x00, sizeof(type_t));
  type->bitfield_size = -1;
  scope_top_obj_insert(cxt, OBJ_TYPE, type);
  return type;
}

// Init a type object from a given object (only shallow copy); Do not set offset field
// Type objects must not be shared because we assign offsets for better error reporting
type_t *type_init_from(type_cxt_t *cxt, type_t *from, char *offset) {
  type_t *ret = type_init(cxt);
  memcpy(ret, from, sizeof(type_t));
  ret->offset = offset;
  ret->bitfield_size = -1;
  return ret;
}

void type_free(void *ptr) {
  type_t *type = (type_t *)ptr;
  if(TYPE_OP_GET(type->decl_prop) == TYPE_OP_FUNC_CALL) {
    list_free(type->arg_list);
    bt_free(type->arg_index);
  }
  free(type);
}

comp_t *comp_init(type_cxt_t *cxt, char *name, char *source_offset, int has_definition) {
  comp_t *comp = (comp_t *)malloc(sizeof(comp_t));
  SYSEXPECT(comp != NULL);
  memset(comp, 0x00, sizeof(comp_t));
  comp->source_offset = source_offset;
  comp->name = name;
  comp->has_definition = has_definition;
  comp->field_list = list_init();
  comp->field_index = bt_str_init();
  if(!has_definition) comp->size = TYPE_UNKNOWN_SIZE; // Forward declaration
  else comp->size = 0;
  scope_top_obj_insert(cxt, OBJ_COMP, comp);
  return comp;
}

void comp_free(void *ptr) {
  comp_t *comp = (comp_t *)ptr;
  list_free(comp->field_list);
  bt_free(comp->field_index);
  free(comp);
}

field_t *field_init(type_cxt_t *cxt) {
  field_t *f = (field_t *)malloc(sizeof(field_t));
  SYSEXPECT(f != NULL);
  memset(f, 0x00, sizeof(field_t));
  scope_top_obj_insert(cxt, OBJ_FIELD, f);
  return f;
}

void field_free(void *ptr) {
  free(ptr);
}

enum_t *enum_init(type_cxt_t *cxt) {
  enum_t *e = (enum_t *)malloc(sizeof(enum_t));
  SYSEXPECT(e != NULL);
  memset(e, 0x00, sizeof(enum_t));
  e->field_list = list_init();
  e->field_index = bt_str_init();
  e->size = TYPE_INT_SIZE;   // Enum always has integer size
  scope_top_obj_insert(cxt, OBJ_ENUM, e);
  return e;
}

void enum_free(void *ptr) {
  enum_t *e = (enum_t *)ptr;
  list_free(e->field_list);
  bt_free(e->field_index);
  free(e);
}

value_t *value_init(type_cxt_t *cxt) {
  value_t *value = (value_t *)malloc(sizeof(value_t));
  SYSEXPECT(value != NULL);
  memset(value, 0x00, sizeof(value_t));
  scope_top_obj_insert(cxt, OBJ_VALUE, value);
  return value;
}

void value_free(void *ptr) {
  value_t *value = (value_t *)ptr;
  if(value->pending_list) list_free(value->pending_list);
  free(ptr);
}

// If the decl node does not have a T_BASETYPE node as first child (i.e. first child T_)
// then the additional basetype node may provide the base type; Caller must free memory
//   1. Do not process storage class including typedef - the caller should add them to symbol table
//      1.1 Type object decl_prop will never have storage class bits set (DECL_STGCLS_MASK)
//   2. Do process struct/union/enum definition
//   3. Type is only valid within the scope it is analyzed
type_t *type_gettype(type_cxt_t *cxt, token_t *decl, token_t *basetype, uint32_t flags) {
  assert(basetype->type == T_BASETYPE);
  token_t *op = ast_getchild(decl, 1);
  token_t *decl_name = ast_getchild(decl, 2);
  assert(decl_name->type == T_ || decl_name->type == T_IDENT);
  decl_prop_t basetype_type = BASETYPE_GET(basetype->decl_prop); // The type primitive of base type decl; only valid with BASETYPE_*
  type_t *curr_type;
  if(!(flags & TYPE_ALLOW_STGCLS) && (basetype->decl_prop & DECL_STGCLS_MASK)) {
    error_row_col_exit(basetype->offset, "Storage class modifier is not allowed in this context\n");
  } else if(!(flags & TYPE_ALLOW_QUAL) && (basetype->decl_prop & DECL_QUAL_MASK)) {
    error_row_col_exit(basetype->offset, "Type qualifier is not allowed in this context\n");
  }
  
  if(basetype_type == BASETYPE_STRUCT || basetype_type == BASETYPE_UNION) {
    token_t *su = ast_getchild(basetype, 0);
    assert(su && (su->type == T_STRUCT || su->type == T_UNION));
    curr_type = type_init(cxt);
    curr_type->offset = su->offset;
    curr_type->decl_prop = basetype->decl_prop & (~DECL_STGCLS_MASK); // Mask off storage class, only copies qualifier
    // If there is no name and no derivation and no body (checked inside type_getcomp) then this is forward
    curr_type->comp = type_getcomp(cxt, su, decl_name->type == T_ && op->type == T_); 
    curr_type->size = curr_type->comp->size; // Could be unknown size
  } else if(basetype_type == BASETYPE_ENUM) {
    token_t *enum_token = ast_getchild(basetype, 0);
    assert(enum_token && enum_token->type == T_ENUM);
    curr_type = type_init(cxt);
    curr_type->offset = enum_token->offset;
    curr_type->decl_prop = basetype->decl_prop & (~DECL_STGCLS_MASK); // Mask off storage class, only copies qualifier
    curr_type->enu = type_getenum(cxt, enum_token); 
    assert(curr_type->enu->size == TYPE_INT_SIZE);
    curr_type->size = TYPE_INT_SIZE;
  } else if(basetype_type == BASETYPE_UDEF) { // Just directly use the udef'ed type
    token_t *udef_name = ast_getchild(basetype, 0);
    assert(udef_name && udef_name->type == T_IDENT);
    curr_type = (type_t *)scope_search(cxt, SCOPE_UDEF, udef_name->str); // May return a struct with or without def
    assert(curr_type); // Must exist because otherwise parser will not tag this as UDEF name
  } else { // This branch is for primitive base types
    if(basetype_type == BASETYPE_VOID) {
      // const void * is also illegal
      if(DECL_STGCLS_MASK & basetype->decl_prop) { error_row_col_exit(basetype->offset, "\"void\" type cannot have storage class\n"); }
      else if(DECL_QUAL_MASK & basetype->decl_prop) { error_row_col_exit(basetype->offset, "\"void\" type cannot have qualifiers\n"); }
      else if(!(flags & TYPE_ALLOW_VOID) && op->type == T_) { 
        // void base type, and no derivation (we allow void * and void function)
        error_row_col_exit(basetype->offset, "\"void\" type can only be used in function argument and return value\n");
      }
      curr_type = type_init_from(cxt, &type_builtin_void, basetype->offset);
    } else if(basetype_type >= BASETYPE_CHAR && basetype_type <= BASETYPE_ULLONG) {
      curr_type = type_init_from(cxt, &type_builtin_ints[BASETYPE_INDEX(basetype_type)], basetype->offset);
      curr_type->decl_prop |= (basetype->decl_prop & DECL_QUAL_MASK); // Also copy const/volatile
    } else {
      type_error_not_supported(basetype->offset, basetype_type);
    }
  }

  while(op->type != T_) {
    assert(op && (op->type == EXP_DEREF || op->type == EXP_FUNC_CALL || op->type == EXP_ARRAY_SUB));
    type_t *parent_type = type_init(cxt);
    parent_type->offset = op->offset;
    parent_type->next = curr_type;
    parent_type->decl_prop = op->decl_prop;       // This copies pointer qualifier (const, volatile)
    assert(!(op->decl_prop & DECL_STGCLS_MASK)); // Must not have storage class (syntax does not allow)
    if(op->type == EXP_DEREF) {
      parent_type->decl_prop |= TYPE_OP_DEREF;
      parent_type->size = TYPE_PTR_SIZE;
    } else if(op->type == EXP_ARRAY_SUB) {
      parent_type->decl_prop |= TYPE_OP_ARRAY_SUB;
      token_t *index = ast_getchild(op, 1);
      assert(index);
      if(index->type != T_) {
        value_t *array_size_value = eval_const_exp(cxt, index);
        if(!type_is_int(array_size_value->type)) {
          error_row_col_exit(index->offset, "Array size in declaration must be of integer type\n");
        } else if(array_size_value->uint32 != array_size_value->uint64) {
          error_row_col_exit(index->offset, "Array size too large to be represented by 32 bit int\n");
        } else if(array_size_value->int32 < 0) {
          error_row_col_exit(index->offset, "Array size in declaration must be non-negative\n");
        }
        op->array_size = array_size_value->int32; // Only use low 31 bits
      } else { op->array_size = -1; }
      parent_type->array_size = op->array_size;
      // If lower type is unknown size (e.g. another array or struct without definition), or the array size not given 
      // then current size is also unknown
      if(op->array_size == -1 || curr_type->size == TYPE_UNKNOWN_SIZE) parent_type->size = TYPE_UNKNOWN_SIZE;
      else parent_type->size = curr_type->size * (size_t)op->array_size;
    } else if(op->type == EXP_FUNC_CALL) {
      // Do not allow const return value
      if((curr_type->decl_prop & DECL_CONST_MASK) || (curr_type->decl_prop & DECL_VOLATILE_MASK)) 
        error_row_col_exit(curr_type->offset, "Function call return value cannot be const or volatile\n");
      parent_type->decl_prop |= TYPE_OP_FUNC_CALL;
      parent_type->size = TYPE_FUNC_SIZE; // Function object is different from function pointer
      parent_type->arg_list = list_init();
      parent_type->arg_index = bt_str_init();
      type_t *arg_type;
      token_t *arg_decl = ast_getchild(op, 1);
      int arg_num = 0;
      while(arg_decl) {
        assert(arg_decl->type == T_DECL || arg_decl->type == T_ELLIPSIS);
        arg_num++;
        if(arg_decl->type == T_ELLIPSIS) {
          if(arg_decl->sibling) 
            error_row_col_exit(op->offset, "\"...\" must be the last argument in function prototype\n")
          parent_type->vararg = 1;
          break; // Must be the last arg, exit loop here
        }
        token_t *arg_basetype = ast_getchild(arg_decl, 0);
        token_t *arg_exp = ast_getchild(arg_decl, 1);
        token_t *arg_name = ast_getchild(arg_decl, 2);
        // This allows both base and decl to be void type
        arg_type = type_gettype(cxt, arg_decl, arg_basetype, TYPE_ALLOW_VOID | TYPE_ALLOW_QUAL); 
        // Detect whether the type is void. (1) Ignore if it is the first and only arg; (2) Otherwise throw error
        if(BASETYPE_GET(arg_type->decl_prop) == BASETYPE_VOID) {
          if(arg_num > 1 || arg_decl->sibling) { error_row_col_exit(op->offset, "\"void\" must be the first and only argument\n"); }
          else if(arg_name->type != T_) { error_row_col_exit(op->offset, "\"void\" argument must be anonymous\n"); }
          else break;
        }
        if(arg_name->type != T_) { // Insert into the index if the arg has a name
          void *bt_ret = bt_insert(parent_type->arg_index, arg_name->str, arg_type);
          if(bt_ret != arg_type) error_row_col_exit(op->offset, "Duplicated argument name \"%s\"\n", arg_name->str);
        }
        list_insert(parent_type->arg_list, arg_name->str, arg_type); // May insert NULL as key
        arg_decl = arg_decl->sibling;
      }
    } // if(current op is function call)
    curr_type = parent_type;
    op = ast_getchild(op, 0); // To the next op (i.e. the op that should be eval'ed before current one)
  } // while(not at the end of AST)

  assert(curr_type);
  return curr_type;
}

// Input must be T_STRUCT or T_UNION
// This function may add new symbol to the current scope
// 1. Has name has body -> normal struct declaration, may optinally also define var
//   1.1 Symbol table already contains the entry, with definition -> name clash, report error
//   1.2 Symbol table already contains the entry, without definition -> Must have seen a forward decl; Use the entry
//   1.3 Symbol table does not contain the entry -> Add to symbol table
// 2. No name, just body -> Anonymous struct declctation, do not add to symbol table
// 3. Just name, no body, used to define var -> Query symbol table
// 4. Just name, no body, do not define var -> Forward declaration, insert a placeholder to symbol table
comp_t *type_getcomp(type_cxt_t *cxt, token_t *token, int is_forward) {
  assert(token->type == T_STRUCT || token->type == T_UNION);
  token_t *name = ast_getchild(token, 0);
  token_t *entry = ast_getchild(token, 1);
  assert(name && entry); // Both children must exist
  int has_name = name->type != T_;
  int has_body = entry->type != T_ || (token->decl_prop & TYPE_EMPTY_BODY); // Has at least one entry, or empty body
  assert(has_name || has_body); // Parser ensures this
  int domain = (token->type == T_STRUCT) ? SCOPE_STRUCT : SCOPE_UNION;
  comp_t *comp = NULL; // If set then do not alloc new
  if(has_name && !has_body) { 
    comp_t *earlier_comp = (comp_t *)scope_search(cxt, domain, name->str); // May return a struct with or without def
    if(!earlier_comp) {
      if(!is_forward) { error_row_col_exit(token->offset, "Struct or union not yet defined: %s\n", name->str); } // Case 3
      else { scope_top_insert(cxt, domain, name->str, earlier_comp = comp_init(cxt, name->str, name->offset, COMP_NO_DEFINITION)); } // Case 4
    }
    return earlier_comp;
  } else if(has_name && has_body) { // Case 1.1 - Case 1.3
    comp_t *ht_ret = (comp_t *)scope_top_find(cxt, domain, name->str); // Only collide with current level
    if(!ht_ret) {
      comp = comp_init(cxt, name->str, name->offset, COMP_HAS_DEFINITION);
      scope_top_insert(cxt, domain, name->str, comp); // Case 1.3
    } else { // Insert here before processing fields s.t. we can include pointer to itself
      if(ht_ret->has_definition) { // Case 1.1
        error_row_col_exit(token->offset, "Redefinition of struct or union: %s\n", name->str);
      } else { // Case 1.2
        comp = ht_ret; 
        comp->has_definition = 1;
      }
    }
  } else if(!has_name && has_body) { // Case 2
    comp = comp_init(cxt, NULL, token->offset, COMP_HAS_DEFINITION);
  }

  if(token->decl_prop & TYPE_EMPTY_BODY) { // If the comp has no body then there is nothing we need to do
    comp->size = 0;
    return comp;
  }

  int curr_offset = 0; // Current entry offset (always 0 for unions)
  size_t max_size = 0;    // Maximum member, only used for unions
  field_t *prev_field = NULL; // Used to coalesce bitfield; Declare outermost because it can take effect across declarations
  while(entry) { 
    assert(entry->type == T_COMP_DECL);
    token_t *basetype = ast_getchild(entry, 0); // This will be repeatedly used
    assert(basetype->type == T_BASETYPE);
    token_t *field = ast_getchild(entry, 1);
    int field_count = 0;
    while(field) {
      field_count++;
      assert(field->type == T_COMP_FIELD);
      token_t *decl = ast_getchild(field, 0);
      assert(decl->type == T_DECL);
      field_t *f = field_init(cxt);
      f->type = type_gettype(cxt, decl, basetype, TYPE_ALLOW_QUAL); // Set field type; do not allow void and storage class
      token_t *field_name = ast_getchild(decl, 2);
      if(field_name->type == T_IDENT) {
        f->name = field_name->str;             // Set field name if there is one
        f->source_offset = field_name->offset; // Set field offset to the name for error reporting
      } else { f->source_offset = field->offset; } // If anonymous field, set offset from the field token
      token_t *bf = ast_getchild(field, 1); // Set bit field (2nd child of T_COMP_FIELD)
      
      if(bf != NULL) {
        assert(bf->type == T_BITFIELD);
        if(!type_is_int(f->type))  // Check whether base type is integer
          error_row_col_exit(bf->offset, "Bit field can only be defined with integers\n");
        value_t *bf_size_value = eval_const_exp(cxt, ast_getchild(bf, 0));
        if(!type_is_int(bf_size_value->type)) {
          error_row_col_exit(bf->offset, "Bit field size in declaration must be of integer type\n");
        } else if(bf_size_value->uint32 != bf_size_value->uint64) {
          error_row_col_exit(bf->offset, "Bit field size too large to be represented by 32 bit int\n");
        } else if(bf_size_value->int32 < 0) {
          error_row_col_exit(bf->offset, "Bit field size in declaration must be non-negative\n");
        }
        field->bitfield_size = bf_size_value->int32;       // Evaluate the constant expression
        f->bitfield_size = field->bitfield_size; 
        f->type->bitfield_size = f->bitfield_size;
        TYPE_OP_SET(f->type->decl_prop, TYPE_OP_BITFIELD); // Indicate in the type that it is a bit field
        if((size_t)f->bitfield_size > f->type->size * 8) 
          error_row_col_exit(field->offset, "Bit field \"%s\" size (%lu bits) must not exceed the integer size (%lu bits)\n", 
            type_printable_name(f->name), (size_t)f->bitfield_size, f->type->size * 8);
      } else { 
        assert(field->bitfield_size == -1);
        f->bitfield_size = f->bitfield_offset = -1; 
        // Do not set this, because it will overwrite array size
        //f->type->bitfield_size = f->type->bitfield_offset = -1; // Also set the value in type object
      }
      
      f->offset = curr_offset; // Set size and offset (currently no alignment)
      f->size = f->type->size;
      if(f->size == TYPE_UNKNOWN_SIZE) // If there is no name then the T_COMP_FIELD has no offset
        error_row_col_exit(f->name ? f->source_offset : basetype->offset, 
          "Struct or union member \"%s\" is incomplete type\n", type_printable_name(f->name));
      
      if(f->name) { // Only insert if there is a name; 
        if(bt_insert(comp->field_index, f->name, f) != f) {
          error_row_col_exit(field_name->offset, "Duplicated field name \"%s\" in composite type declaration\n", f->name);
        }
        list_insert(comp->field_list, f->name, f); // Named field, insert
      } else { // Promote inner composite type names to the current scope
        assert(field_count == 1);   // If there is no name it must be the first field
        if(type_is_comp(f->type)) { // Anonymous comp field, promote, with the type node's qualifier (no storage class)
          if(f->type->comp->name)   // Could not declare struct { struct named_struct { ... } ; } which is confusing
            error_row_col_exit(f->type->comp->source_offset, "Please do not declare named struct with anonymous variables\n");
          decl_prop_t qual = f->type->decl_prop & DECL_QUAL_MASK; // OR'ed onto every field's decl prop
          listnode_t *promote_head = list_head(f->type->comp->field_list);
          while(promote_head) {
            char *promote_name = list_key(promote_head);
            field_t *promote_field = list_value(promote_head);
            if(promote_name) {
              if(bt_insert(comp->field_index, promote_name, promote_field) != promote_field) {
                error_row_col_exit(promote_field->source_offset, 
                  "Promoted anonymous composite field name \"%s\" clashes with including type\n", promote_name);
              }
            }
            promote_field->type->decl_prop |= qual; // Inherit from including comp type's qualifiers
            promote_field->offset += curr_offset;   // Offset by the current position
            list_insert(comp->field_list, promote_name, promote_field);
            promote_head = list_next(promote_head);
          }
        } else { list_insert(comp->field_list, NULL, f); } // Anonymous non-comp field, insert
      }
      
      if(token->type == T_STRUCT) {
        //printf("%s %d\n", f->name, f->bitfield_size);
        // Non-Bit field always increments the offset; This works also for promoted types
        // Bit field coalesce rules:
        // (1) Adjacent entries of the same integer type will be coalesced; If they cannot be packed into the 
        //     declared base type, we leave a gap and start a new field;
        // (2) Different types (incl. sign difference) are never coalesced;
        // (3) Packed fields are arranged from lower bits to higher bits; Unused bits will be ignored
        // (4) Coalesced bit fields take the storage identical to the base type
        if(!prev_field) { // First entry in struct
          if(f->bitfield_size != -1) f->type->bitfield_offset = f->bitfield_offset = 0;
          curr_offset += f->type->size;
        } else if(f->bitfield_size == -1 && prev_field->bitfield_size == -1) {
          curr_offset += f->type->size; // Both normal fields; size has been set above
        } else if(f->bitfield_size == -1 && prev_field->bitfield_size != -1) {
          curr_offset += f->type->size;
        } else if(f->bitfield_size != -1 && prev_field->bitfield_size == -1) {
          f->type->bitfield_offset = f->bitfield_offset = 0; // Starting a bit field (may potentially have more later)
          curr_offset += f->type->size;
        } else { // Both are valid bitfields - try to coalesce!
          assert(f->bitfield_size != -1 && prev_field->bitfield_size != -1);
          //printf("%s %s %d %d\n", prev_field->name, f->name, prev_field->bitfield_size, f->bitfield_size);
          // If types differ, start a new bit field
          if(BASETYPE_GET(prev_field->type->decl_prop) != BASETYPE_GET(f->type->decl_prop)) {
            f->type->bitfield_offset = f->bitfield_offset = 0;
            curr_offset += prev_field->type->size;
          } else if(prev_field->bitfield_offset + prev_field->bitfield_size + f->bitfield_size <= (int)prev_field->size * 8) {
            f->offset = prev_field->offset; 
            f->type->bitfield_offset = f->bitfield_offset = prev_field->bitfield_offset + prev_field->bitfield_size;
          } else {
            f->offset = prev_field->offset + prev_field->type->size; 
            f->type->bitfield_offset = f->bitfield_offset = 0;
            curr_offset += f->type->size;
          }
        }
      } else if(max_size < f->type->size) {
        max_size = f->type->size;
      }
      
      field = field->sibling;
      prev_field = f;
    } // while(field)
    entry = entry->sibling;
  }
  if(token->type == T_STRUCT) comp->size = (size_t)curr_offset;
  else comp->size = max_size;                            // Size of union is the maximum of all types
  return comp;
}

enum_t *type_getenum(type_cxt_t *cxt, token_t *token) {
  assert(token->type == T_ENUM);
  enum_t *enu = enum_init(cxt);
  enu->offset = token->offset;
  token_t *name = ast_getchild(token, 0);
  token_t *field = ast_getchild(token, 1);
  assert(name);
  int nameless = name->type == T_;
  if(!nameless) enu->name = name->str;
  int curr_value = 0;
  while(field) {
    assert(field->type == T_ENUM_FIELD);
    token_t *entry_name = ast_getchild(field, 0);
    assert(entry_name->type == T_IDENT); // Enum field must have a name
    token_t *enum_exp = ast_getchild(field, 1);
    if(enum_exp) {
      value_t *enum_value = eval_const_exp(cxt, enum_exp);
      if(!type_is_int(enum_value->type)) {
        error_row_col_exit(enum_exp->offset, "Enum constant must be of integer type\n");
      } else if(enum_value->uint32 != enum_value->uint64) {
        error_row_col_exit(enum_exp->offset, "Enum constant value too large to be represented by 32 bit int\n");
      } 
      field->enum_const = curr_value = enum_value->int32;
    } else {
      field->enum_const = curr_value;
    }
    char *name_str = entry_name->str;
    list_insert(enu->field_list, name_str, (void *)(long)curr_value); // Directly store the integer as value
    if(bt_find(enu->field_index, name_str) != BT_NOTFOUND) {
      error_row_col_exit(field->offset, "Enum field name \"%s\" clashes with a previous name\n", name_str);
    } else {
      bt_insert(enu->field_index, name_str, (void *)(long)curr_value);
    }
    if(nameless) { // Insert the names to the current scope as integer const
      value_t *value = value_init(cxt);
      value->addrtype = ADDR_IMM;
      value->int32 = curr_value;
      // Assign built in type, do not create new type objects. Offset is the offset of symbol name
      value->type = type_init_from(cxt, &type_builtin_ints[BASETYPE_INDEX(BASETYPE_INT)], entry_name->offset);
      if(scope_top_find(cxt, SCOPE_VALUE, name_str)) {
        error_row_col_exit(entry_name->offset, "Anonymous enum field \"%s\" clashes with an existing name", name_str);
      } else {
        scope_top_insert(cxt, SCOPE_VALUE, name_str, value);
      }
    }
    field = field->sibling;
    curr_value++;
  }
  return enu;
}

// Integer operation result type is a function of both operands
//   1. Size of the result is always the larger of them
//   2. If both are signed or unsigned the result is also consistent
//   3. Otherwise, if one type is longer, then use the longer type's sign
//   4. If two types are of equal length, and one is unsigned, the result is unsigned
type_t *type_int_convert(type_t *lhs, type_t *rhs) {
  decl_prop_t int1 = lhs->decl_prop;
  decl_prop_t int2 = rhs->decl_prop;
  assert(BASETYPE_GET(int1) >= BASETYPE_CHAR && BASETYPE_GET(int1) <= BASETYPE_ULLONG);
  assert(BASETYPE_GET(int2) >= BASETYPE_CHAR && BASETYPE_GET(int2) <= BASETYPE_ULLONG);
  int_prop_t p1 = ints[BASETYPE_INDEX(int1)], p2 = ints[BASETYPE_INDEX(int2)];
  // MIN on sign means that we prefer unsigned when the sizes are equal
  int_prop_t ret = {EVAL_MIN(p1.sign, p2.sign), EVAL_MAX(p1.size, p2.size)}; 
  // The sign of the longer type override the sign of shorter type
  if(p1.size > p2.size) ret.sign = p1.sign;
  else if(p2.size > p1.size) ret.sign = p2.sign;
  for(int i = 1;i < (int)sizeof(ints) / (int)sizeof(int_prop_t);i++) 
    if(memcmp(&ints[i], &ret, sizeof(int_prop_t)) == 0) return &type_builtin_ints[i];
  assert(0); // Cannot reach here
  return 0;
}

// Compare two types
//   1. If the two types are exactly identical, return TYPE_CMP_EQ
//   2. If the two types only differ by const and/or volatile qualifier, and the const/volatile is
//      on the "to" type, return TYPE_CMP_LOSELESS (do not lose information by casting to a more 
//      strict type)
//      Note: It is possible to implicitly cast a less strict type to more strict type, e.g. const int -> int
//      is generally fine
//   3. If the two types only differ by const and/or volatile qualifier, and the qualifier is on
//      "from" type, return TYPE_CMP_LOSSY
//   4. If two types differ in their derivation chain and/or base types, return TYPE_CMP_NEQ
// This function does not treat array and deref as the same type; Caller should be aware
int type_cmp(type_t *to, type_t *from) {
  decl_prop_t base1, base2;
  int const_flag, volatile_flag, lossy_flag, eq_flag;
  decl_prop_t op1 = TYPE_OP_GET(to->decl_prop);
  decl_prop_t op2 = TYPE_OP_GET(from->decl_prop);
  // Reject if derivations are different
  if(op1 != op2) return TYPE_CMP_NEQ;

  // If set then const and volatile are incompatible
  // All derivations and base types may have a qualifier: base type, *, [], ()
  const_flag = !(to->decl_prop & DECL_CONST_MASK) && (from->decl_prop & DECL_CONST_MASK);
  volatile_flag = !(to->decl_prop & DECL_VOLATILE_MASK) && (from->decl_prop & DECL_VOLATILE_MASK);
  lossy_flag = const_flag || volatile_flag; // Set to 1 if RHS is more strict than LHS
  eq_flag = (to->decl_prop & DECL_QUAL_MASK) == (from->decl_prop & DECL_QUAL_MASK);
  //printf("eq flag = %d from %s to %s\n", eq_flag, type_print_str(0, from, NULL, 0), type_print_str(1, to, NULL, 0));
  assert(!eq_flag || !lossy_flag); // At most one can be 1

  // Base type, end of recursion
  if(op1 == TYPE_OP_NONE) {
    base1 = BASETYPE_GET(to->decl_prop);
    base2 = BASETYPE_GET(from->decl_prop);
    if(base1 != base2) return TYPE_CMP_LOSSY; // Reject different base types
    if(base1 == BASETYPE_STRUCT || base1 == BASETYPE_UNION) {
      // Note that struct, union and enum are only created once and used multiple times, so just compare ptr
      if(to->comp == from->comp) return eq_flag ? TYPE_CMP_EQ : (lossy_flag ? TYPE_CMP_LOSSY : TYPE_CMP_LOSELESS);
      else return TYPE_CMP_NEQ; // Different structs
    } else if(base1 == BASETYPE_ENUM) {
      if(to->enu == from->enu) return eq_flag ? TYPE_CMP_EQ : (lossy_flag ? TYPE_CMP_LOSSY : TYPE_CMP_LOSELESS);
      else return TYPE_CMP_NEQ;
    } else { // Base type, must be identical, so just check lossy flag
      return eq_flag ? TYPE_CMP_EQ : (lossy_flag ? TYPE_CMP_LOSSY : TYPE_CMP_LOSELESS);
    }
  } else {
    assert(op1 == TYPE_OP_DEREF || op1 == TYPE_OP_ARRAY_SUB || op1 == TYPE_OP_FUNC_CALL);
    if(op1 == TYPE_OP_ARRAY_SUB) { // Compare index - if none then it will be -1 which fits naturally into this
      if(to->array_size != from->array_size) return TYPE_CMP_NEQ;
    } else if(op1 == TYPE_OP_FUNC_CALL) { // Compare argument - must be strictly identical, not even compatible
      if(to->vararg != from->vararg) return TYPE_CMP_NEQ; // Vararg functions must match
      if(list_size(to->arg_list) != list_size(from->arg_list)) return TYPE_CMP_NEQ; // Argument number
      listnode_t *to_arg = list_head(to->arg_list);
      listnode_t *from_arg = list_head(from->arg_list);
      while(to_arg && from_arg) {
        int ret = type_cmp((type_t *)list_value(to_arg), (type_t *)list_value(from_arg));
        if(ret != TYPE_CMP_EQ) return TYPE_CMP_NEQ; // Args must be strictly equal
        to_arg = list_next(to_arg);
        from_arg = list_next(from_arg);
      }
    }
    int ret = type_cmp(to->next, from->next); 
    if(op1 != TYPE_OP_FUNC_CALL) {
      switch(ret) {
        case TYPE_CMP_NEQ: return TYPE_CMP_NEQ; break;
        case TYPE_CMP_LOSSY: return TYPE_CMP_LOSSY; break;
        case TYPE_CMP_LOSELESS: return eq_flag ? TYPE_CMP_LOSELESS : (lossy_flag ? TYPE_CMP_LOSSY : TYPE_CMP_LOSELESS);
        case TYPE_CMP_EQ: return eq_flag ? TYPE_CMP_EQ : (lossy_flag ? TYPE_CMP_LOSSY : TYPE_CMP_LOSELESS);
        default: assert(0);
      }
    } else { // For func arg this compares the return value, and if it is not EQ then it is NEQ
      if(ret == TYPE_CMP_EQ) return TYPE_CMP_EQ;
      else return TYPE_CMP_NEQ;
    }
  }
  assert(0);
  return 0;
}

// Performs type cast:
//   1. Explicit type cast: Using EXP_CAST operator
//   2. Implicit type cast: Assignments, array indexing, function arguments
// Type cast rule:
//   0. any type <-> itself
//   1. int <-> int (both)
//   2. int <-> ptr (explicit)
//   3. ptr <-  array of the same base type (both)
//      Note: ptr cannot be casted to arrays in any context; For function args array is treated as a pointer
//   4. ptr <-> ptr (see below)
//   5. Any type to void is allowed, returns TYPE_CAST_VOID
//   6. Function type to pointer of the same function type
//   7. Could not cast void to any type
// Implicit cast rules:
//   1.1 Implicit cast does not allow casting longer integer to shorter integer
//   1.2 Casting signed shorter int to longer types always use sign extension
//   2.1 Do not allow casting pointers from/to different sized integer types
//   4.1 Implicit cast does not allow casting between pointers, except to and from void * type
//       4.1.1 But, if these two pointed to types only differ by const, then we can implicitly cast non-const 
//             to const; Same applies to volatile
//   *.* Casting from const to non-const implicitly is prohibited for all types
// See TYPE_CAST_ series for return values
// This function will report error and exit if an error is detected
int type_cast(type_t *to, type_t *from, int cast_type, char *offset) {
  assert(cast_type == TYPE_CAST_EXPLICIT || cast_type == TYPE_CAST_IMPLICIT);
  // Handle easy cases first:
  if(type_is_void(from)) {  // Case 7: from void
    error_row_col_exit(offset, "Casting void type is disallowed\n"); 
  } else if(type_cmp(to, from) == TYPE_CMP_EQ) { // Case 0: self-cast
    return TYPE_CAST_NO_OP; 
  } else if(type_is_void(to)) { // Case 5: to void
    return TYPE_CAST_VOID; 
  }

  if(type_is_int(to) && type_is_int(from)) { // Case 1: int to int
    if(cast_type == TYPE_CAST_EXPLICIT) {
      if(to->size == from->size) return TYPE_CAST_NO_OP;        // Same size cast - always allowed for explicit casting
      else if(to->size < from->size) return TYPE_CAST_TRUNCATE; // Casting from longer to shorter
      if(type_is_signed(from)) return TYPE_CAST_SIGN_EXT;       // Casting from shorter signed int
      else return TYPE_CAST_ZERO_EXT;                           // Casting from shorter unsigned int
    } else {
      int from_sign = type_is_signed(from);
      int to_sign = type_is_signed(to);
      if(to->size < from->size)
        error_row_col_exit(offset, "Cannot cast from longer integer type to shorter integer type implicitly\n"); 
      if(to->size == from->size) { // No bit change
        return TYPE_CAST_NO_OP;
      }
      // To longer type - same as explicit
      if(from_sign) return TYPE_CAST_SIGN_EXT;
      else return TYPE_CAST_ZERO_EXT;
    }
  } else if(type_is_int(to) && type_is_ptr(from)) { // Case 2, one direction
    if(cast_type != TYPE_CAST_EXPLICIT) 
      error_row_col_exit(offset, "Could not cast pointer to integer implicitly\n");
    if(type_get_int_size(to) != TYPE_PTR_SIZE) 
      error_row_col_exit(offset, "Could not cast pointer to integer of different sizes\n");
    return TYPE_CAST_NO_OP;
  } else if(type_is_ptr(to) && type_is_int(from)) { // Case 2, the other direction
    if(cast_type != TYPE_CAST_EXPLICIT) 
      error_row_col_exit(offset, "Could not cast integer to pointer implicitly\n");
    if(type_get_int_size(from) != TYPE_PTR_SIZE) 
      error_row_col_exit(offset, "Could not cast integer to pointer of different sizes\n");
    return TYPE_CAST_NO_OP;
  } else if(type_is_ptr(to) && type_is_array(from)) { // Case 3
    if(type_is_void_ptr(to)) return TYPE_CAST_GEN_PTR; // Case 5
    if(cast_type == TYPE_CAST_IMPLICIT) { // Only check pointed type for compatibility if it is implicit
      int ret = type_cmp(to->next, from); // i.e. to->next should be an array type
      if(ret == TYPE_CMP_NEQ) { error_row_col_exit(offset, "Cannot cast array to pointer type of different base types\n"); }
      else if(ret == TYPE_CMP_LOSSY) { error_row_col_exit(offset, "Cannot cast array to incompatible pointer type\n"); }
    }
    return TYPE_CAST_GEN_PTR;
  } else if(type_is_ptr(to) && type_is_ptr(from)) { // Case 4
    if(type_is_void_ptr(to) || type_is_void_ptr(from)) return TYPE_CAST_NO_OP; // Case 4.1
    if(cast_type == TYPE_CAST_IMPLICIT) { // Only check pointed type for compatibility if it is implicit
      int ret = type_cmp(to->next, from->next);
      if(ret == TYPE_CMP_NEQ) { error_row_col_exit(offset, "Cannot cast between pointers of different base types\n"); }
      else if(ret == TYPE_CMP_LOSSY) { error_row_col_exit(offset, "Cannot cast pointer to incompatible base types\n"); }
    }
    return TYPE_CAST_NO_OP;
  } else if(type_is_ptr(to) && type_is_func(from)) { // Case 6
    if(type_is_void_ptr(to)) return TYPE_CAST_GEN_PTR; // Case 5
    if(cast_type == TYPE_CAST_IMPLICIT) { // Only check pointed type for compatibility if it is implicit
      int ret = type_cmp(to->next, from); // Note: We compare the function type with the pointer's target type
      assert(ret == TYPE_CMP_EQ || ret == TYPE_CMP_NEQ); // Function type can only return these two results
      if(ret == TYPE_CMP_NEQ) error_row_col_exit(offset, "Cannot cast function to pointer type of a different prototype\n");
    }
    return TYPE_CAST_GEN_PTR;
  }
  // All other casts are invalid
  error_row_col_exit(offset, "Invalid %s type cast\n", cast_type == TYPE_CAST_EXPLICIT ? "explicit" : "implicit");
  assert(0);
  return TYPE_CAST_INVALID;
}

// This function evaluates the type of an expression
// Argument options: see TYPEOF_IGNORE_ series. For functions and arrays we do not need the type of 
// arguments and index to determine the final type; Caller must not modify the returned type
//   1. For literal types, just return their type constant
//   2. void can be the result of casting, and can be returned
//   3. Bit fields within a struct returns the base element type (e.g. For struct { short int x : 15; }, x type is "int")
type_t *type_typeof(type_cxt_t *cxt, token_t *exp, uint32_t options) {
  // Leaf types: Integer literal, string literal and identifiers
  if(BASETYPE_GET(exp->decl_prop) >= BASETYPE_CHAR && BASETYPE_GET(exp->decl_prop) <= BASETYPE_ULLONG) {
    return type_init_from(cxt, &type_builtin_ints[BASETYPE_INDEX(exp->decl_prop)], exp->offset);
  } else if(exp->type == T_STR_CONST) {
    str_t *s = eval_const_str_token(exp);
    size_t sz = str_size(s);
    str_free(s);                             // Do not use its content
    return type_get_strliteral(cxt, sz + 1, exp->offset); // We reserve one byte for trailing '\0'
  } else if(BASETYPE_GET(exp->decl_prop)) {  // Unsupported base type literal
    type_error_not_supported(exp->offset, exp->decl_prop);
  } else if(exp->type == T_IDENT) {
    value_t *value = scope_search(cxt, SCOPE_VALUE, exp->str);
    if(!value) error_row_col_exit(exp->offset, "Name \"%s\" does not exist in current scope\n", exp->str);
    return value->type;
  }

  token_type_t op_type = exp->type;
  type_t *lhs, *rhs;
  // Type derivation operators: * -> . () []
  if(op_type == EXP_DEREF) { // Dereference can be applied to both ptr and array type
    lhs = type_typeof(cxt, ast_getchild(exp, 0), options);
    if(TYPE_OP_GET(lhs->decl_prop) != TYPE_OP_DEREF && TYPE_OP_GET(lhs->decl_prop) != TYPE_OP_ARRAY_SUB) 
      error_row_col_exit(exp->offset, "Operator \'*\' cannot be applied to non-pointer (or array) type\n");
    return lhs->next;
  } else if(op_type == EXP_ARRAY_SUB) {
    lhs = type_typeof(cxt, ast_getchild(exp, 0), options);
    if(TYPE_OP_GET(lhs->decl_prop) != TYPE_OP_DEREF && TYPE_OP_GET(lhs->decl_prop) != TYPE_OP_ARRAY_SUB) 
      error_row_col_exit(exp->offset, "Operator \'[]\' cannot be applied to non-array (or pointer) type\n");
    if(!(options & TYPEOF_IGNORE_ARRAY_INDEX)) {
      token_t *index_token = ast_getchild(exp, 1);
      assert(index_token);
      type_t *index_type = type_typeof(cxt, index_token, options);
      if(!type_is_int(index_type)) 
        error_row_col_exit(index_token->offset, "Array index must be of one of the integral types\n");
    }
    return lhs->next;
  } else if(op_type == EXP_FUNC_CALL) { // Function call operator will dereference the ptr implicitly
    token_t *func_token = ast_getchild(exp, 0);
    lhs = type_typeof(cxt, func_token, options);
    if(!type_is_func(lhs) && !type_is_func_ptr(lhs)) 
      error_row_col_exit(func_token->offset, "Function call must be applied to function of function pointer\n");
    if(type_is_func_ptr(lhs)) lhs = lhs->next;
    // Invariant: after this line, lhs is always function call type
    if(!(options & TYPEOF_IGNORE_FUNC_ARG)) {
      listnode_t *arg = list_head(lhs->arg_list); // Expected type
      token_t *arg_token;
      int arg_index = 1; // Index of argument starts at 1 under exp node
      while(arg) {
        arg_token = ast_getchild(exp, arg_index); // Actual type
        if(!arg_token) error_row_col_exit(exp->offset, "Missing argument %d in function call\n", arg_index);
        type_t *arg_type = type_typeof(cxt, arg_token, options);
        // This will report error if implicit cast is illegal
        type_cast(list_value(arg), arg_type, TYPE_CAST_IMPLICIT, arg_token->offset); 
        arg = list_next(arg);
        arg_index++;
      }
      // If after expected arg list is exhausted there is still argument expression, we have passed too many args
      if(ast_getchild(exp, arg_index)) error_row_col_exit(arg_token->offset, "Too many arguments to function\n");
    }
    return lhs->next;
  }
  
  // Everything down below must have at least one operand whose type is the first child of exp
  lhs = type_typeof(cxt, ast_getchild(exp, 0), options);
  const char *op_str = token_symstr(exp->type);
  switch(op_type) {
    // If applied to integer then result is the same integer, if applied to pointers then result is pointer
    case EXP_POST_INC: case EXP_PRE_INC: case EXP_PRE_DEC: case EXP_POST_DEC: {
      if(type_is_int(lhs) || type_is_ptr(lhs)) return lhs;
      error_row_col_exit(exp->offset, "Invalid operand for \"%s\" operator\n", op_str);
    } break;
    case EXP_ARROW:
      if(!type_is_ptr(lhs)) error_row_col_exit(exp->offset, "Operator \"->\" must be applied to pointer types\n");
      lhs = lhs->next;
      // Fall thruogh
    case EXP_DOT: {
      if(!type_is_comp(lhs)) 
        error_row_col_exit(exp->offset, "Operator \"%s\" must be applied to composite types\n", op_str);
      comp_t *comp = lhs->comp;
      token_t *field_name_token = ast_getchild(exp, 1);
      assert(field_name_token);
      if(field_name_token->type != T_IDENT) error_row_col_exit(field_name_token->offset, "Invalid field specifier\n");
      char *field_name = field_name_token->str;
      void *ret = bt_find(comp->field_index, field_name);
      if(ret == BT_NOTFOUND) error_row_col_exit(exp->offset, "Composite type has no field \"%s\"\n", field_name);
      return ((field_t *)ret)->type; // If it is a bit field the type object has the field set to -1
    } break;
    case EXP_PLUS: case EXP_MINUS: {
      if(!type_is_int(lhs)) 
        error_row_col_exit(exp->offset, "Operator \"%s\" must be applied to integer types\n", op_str);
      return lhs;
    } break;
    case EXP_LOGICAL_NOT: {
      if(!type_is_int(lhs) && !type_is_ptr(lhs)) 
        error_row_col_exit(exp->offset, "Operator \'!\' must be applied to integer or pointer types\n");
      return lhs;
    } break;
    case EXP_BIT_NOT: {
      if(!type_is_int(lhs)) 
        error_row_col_exit(exp->offset, "Operator \'~\' must be applied to integer types\n");
      return lhs;
    }
    case EXP_CAST: { // For EXP_CAST, lhs is the expression, while RHS is the T_DECL with first child being T_BASETYPE
      token_t *decl_token = ast_getchild(exp, 1);
      token_t *basetype_token = ast_getchild(decl_token, 0);
      // Allow casting to void or functions returning void
      type_t *target = type_gettype(cxt, decl_token, basetype_token, TYPE_ALLOW_VOID | TYPE_ALLOW_QUAL); 
      type_cast(target, lhs, TYPE_CAST_EXPLICIT, exp->offset);
      return target;
    } break;
    case EXP_ADDR: { // This works even for the two symbol types: ARRAY_SUB and FUNC_CALL
      if(lhs->bitfield_size == -1) error_row_col_exit(exp->offset, "Cannot take address of bitfields\n");
      type_t *deref = type_init(cxt);   // Create a new type node
      deref->decl_prop = TYPE_OP_DEREF;
      deref->next = lhs;
      deref->offset = exp->offset;
      deref->size = TYPE_PTR_SIZE;
      return deref;
    } break;
    case EXP_SIZEOF: { // sizeof() operator returns size_t type, which is unsigned long
      return type_init_from(cxt, &type_builtin_ints[BASETYPE_INDEX(TYPE_SIZEOF_TYPE)], exp->offset);
    } break;
    // Group of operators that just perform an integer convert and check feasibility
    // Optionally cast back to the LHS type for assignment
    case EXP_MUL: case EXP_DIV: case EXP_MOD: 
    case EXP_BIT_AND: case EXP_BIT_OR: case EXP_BIT_XOR: 
    case EXP_MUL_ASSIGN: case EXP_DIV_ASSIGN: case EXP_MOD_ASSIGN: 
    case EXP_AND_ASSIGN: case EXP_OR_ASSIGN: case EXP_XOR_ASSIGN: {
      rhs = type_typeof(cxt, ast_getchild(exp, 1), options); // Evaluate both left and right operands
      if(type_is_int(lhs) && type_is_int(rhs)) { // Integer type conversion
        type_t *after_convert = type_int_convert(lhs, rhs);
        // Test whether they could convert, e.g. (int * unsigned int) is invalid because int could not be casted to unsigned int
        type_cast(after_convert, lhs, TYPE_CAST_IMPLICIT, ast_getchild(exp, 0)->offset);
        type_cast(after_convert, rhs, TYPE_CAST_IMPLICIT, ast_getchild(exp, 1)->offset);
        if(token_is_assign(exp)) { // += -= needs to cast result back to lhs
          type_cast(lhs, after_convert, TYPE_CAST_IMPLICIT, exp->offset);
          return lhs;
        } else {
          return after_convert;
        }
      }
      error_row_col_exit(exp->offset, "Operator \"%s\" must be applied to integer types", op_str);
    } break;
    // This group of operators can operate on both pointers and integers
    case EXP_ADD: case EXP_SUB: 
    case EXP_ADD_ASSIGN: case EXP_SUB_ASSIGN: {
      // Copied from above
      rhs = type_typeof(cxt, ast_getchild(exp, 1), options); 
      if(type_is_int(lhs) && type_is_int(rhs)) { 
        type_t *after_convert = type_int_convert(lhs, rhs);
        type_cast(after_convert, lhs, TYPE_CAST_IMPLICIT, ast_getchild(exp, 0)->offset);
        type_cast(after_convert, rhs, TYPE_CAST_IMPLICIT, ast_getchild(exp, 1)->offset);
        if(token_is_assign(exp)) { // += -= needs to cast result back to lhs
          type_cast(lhs, after_convert, TYPE_CAST_IMPLICIT, exp->offset);
          return lhs;
        } else {
          return after_convert;
        }
      } else if(type_is_ptr(lhs) && type_is_int(rhs)) {
        return lhs;
      } else if(op_type == EXP_SUB && type_is_ptr(lhs) && type_is_ptr(rhs)) { // Pointer subtraction
        if(type_is_void_ptr(lhs) || type_is_void_ptr(rhs)) error_row_col_exit(exp->offset,
          "Could not subtract to or from void pointer\n");
        int ret = type_cmp(lhs->next, rhs->next); // Don't care about const/volatile bc they do not affect type size
        if(ret == TYPE_CMP_NEQ) error_row_col_exit(exp->offset, 
          "Pointer subtraction can only be applied to pointers of the same base type\n");
        return type_getint(TYPE_PTR_DIFF_TYPE);
      }
      error_row_col_exit(exp->offset, "Operator \"%s\" cannot be applied here", op_str);
    } break;
    // No extra check for assign because shift operator returns the lhs always
    case EXP_LSHIFT: case EXP_RSHIFT: 
    case EXP_LSHIFT_ASSIGN: case EXP_RSHIFT_ASSIGN: { // Shift operator preserves the type
      rhs = type_typeof(cxt, ast_getchild(exp, 1), options); 
      if(type_is_int(lhs) && type_is_int(rhs)) return lhs;
      error_row_col_exit(exp->offset, 
        "Operator \"%s\" must be applied to integer types", op_str);
    } break;
    case EXP_LESS: case EXP_GREATER: case EXP_LEQ: case EXP_GEQ: 
    case EXP_EQ: case EXP_NEQ: { // Comparison requires the same as +/-
      rhs = type_typeof(cxt, ast_getchild(exp, 1), options); 
      if(type_is_int(lhs) && type_is_int(rhs)) { 
        type_t *after_convert = type_int_convert(lhs, rhs); // Must convert to same length and must not lose information
        type_cast(after_convert, lhs, TYPE_CAST_IMPLICIT, ast_getchild(exp, 0)->offset);
        type_cast(after_convert, rhs, TYPE_CAST_IMPLICIT, ast_getchild(exp, 1)->offset);
      } else if(type_is_ptr(lhs) && type_is_ptr(rhs)) {
        if(!type_is_void_ptr(lhs) && !type_is_void_ptr(rhs)) {
          int ret = type_cmp(lhs->next, rhs->next); // Compare target type of pointers
          if(ret == TYPE_CMP_NEQ) error_row_col_exit(exp->offset, 
            "Pointer comparison must have the same base type (except const/volatile)\n");
        }
      }
      return type_getint(BASETYPE_INT); // Comparison result is always signed int
    } break;
    case EXP_LOGICAL_AND: case EXP_LOGICAL_OR: { // && || accepts both pointer and integer as operands
      rhs = type_typeof(cxt, ast_getchild(exp, 1), options); 
      if(!type_is_int(lhs) && !type_is_ptr(lhs)) 
        error_row_col_exit(ast_getchild(exp, 0)->offset, 
          "Operatpr \"%s\" must be applied to integer or pointer type\n", op_str);
      if(!type_is_int(rhs) && !type_is_ptr(rhs)) 
        error_row_col_exit(ast_getchild(exp, 1)->offset, 
          "Operatpr \"%s\" must be applied to integer or pointer type\n", op_str);
    } break;
    case EXP_COND: { // This operator has three operands. Note that op2 and op3 could have void type
      token_t *op2 = ast_getchild(exp, 1);
      token_t *op3 = ast_getchild(exp, 2);
      assert(op2 && op3);
      if(!type_is_int(lhs) && !type_is_ptr(lhs)) // First check condition
        error_row_col_exit(ast_getchild(exp, 0)->offset, 
          "The first operand of condition operator must be integer or pointer type\n");
      type_t *type2 = type_typeof(cxt, op2, options);
      type_t *type3 = type_typeof(cxt, op3, options);
      int ret = type_cmp(type2, type3);
      if(ret != TYPE_CMP_EQ) // Must be strictly identical, because at run time both could be used as the operand
        error_row_col_exit(exp->offset, 
          "The type of two options in conditional expression must be identical (you may use cast)\n");
      return type2;
    }
    case EXP_ASSIGN: { // All assignments return the type of the left operand
      rhs = type_typeof(cxt, ast_getchild(exp, 1), options); 
      type_cast(lhs, rhs, TYPE_CAST_IMPLICIT, exp->offset); 
      return lhs;
    } break;
    case EXP_COMMA: { // Comma operator only returns the value of the second expression
      // Note that here we also perform type check for LHS by recursively calling this function
      rhs = type_typeof(cxt, ast_getchild(exp, 1), options); 
      return rhs;
    }
    default: assert(0);
  }
  return NULL;
}

