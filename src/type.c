
#include "token.h"
#include "type.h"
#include "eval.h"
#include "ast.h"
#include "str.h"

int_prop_t ints[11] = { // Integer sign and size, using index of base type
  {-1, -1}, // BASETYPE_NONE, 0x00
  {1, 1}, // BASETYPE_CHAR, 0x01
  {1, 2}, // BASETYPE_SHORT, 0x02
  {1, 4}, // BASETYPE_INT, 0x03
  {1, 8}, // BASETYPE_LONG, 0x04
  {0, 1}, // BASETYPE_UCHAR, 0x05
  {0, 2}, // BASETYPE_USHORT, 0x06
  {0, 4}, // BASETYPE_UINT, 0x07
  {0, 8}, // BASETYPE_ULONG, 0x08
  {1, 16}, // BASETYPE_LLONG, 0x09
  {0, 16}, // BASETYPE_ULLONG, 0x0A
};

// Prints a type in string on stdout
// type is the type object, name is shown as the inner most operand of the type expression, NULL means no name
// The top level should call this function with s == NULL. The return value contains the type string
str_t *type_print(type_t *type, const char *name, str_t *s, int print_comp_body, int level) {
  assert(type);
  if(!s) s = str_init();
  for(int i = 0;i < level * 2;i++) str_append(s, ' '); // Padding spaces to the level
  // Print base type first
  type_t *basetype = type;
  while(basetype->next) basetype = basetype->next;
  // Print storage class and qualifier and base type, e.g. struct, union, enum, udef; with a space at the end
  str_concat(s, token_decl_print(basetype->decl_prop));
  // This does not include qualifier and storage class
  decl_prop_t base = BASETYPE_GET(type->decl_prop);
  str_append(s, ' ');
  if(base == BASETYPE_STRUCT || base == BASETYPE_UNION) {
    comp_t *comp = basetype->comp;
    if(comp->name) { str_concat(s, comp->name); str_append(s, ' '); } // Name followed by a space
    if(print_comp_body && comp->has_definition) {
      str_concat(s, "{\n");
      listnode_t *node = list_head(comp->field_list);
      while(node) {
        field_t *field = (field_t *)list_value(node);
        str_t *field_s = type_print(field->type, field->name, NULL, print_comp_body, level + 1);
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
  } else if(base == BASETYPE_UDEF) {
    str_concat(s, type->udef_name);
    str_append(s, ' ');
  } else if(base == BASETYPE_ENUM) {
    // TODO: ADD PRINT FOR ENUM
    assert(0);
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
        str_t *arg_s = type_print(list_value(arg), list_key(arg), NULL, 0, 0);
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

type_cxt_t *type_sys_init() {
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
void *scope_top_insert(type_cxt_t *cxt, int domain, void *key, void *value) { return ht_insert(scope_top_name(cxt, domain), key, value); }

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
  scope_top_obj_insert(cxt, OBJ_TYPE, type);
  return type;
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
  memset(e, 0x00, sizeof(enum_t));
  e->field_list = list_init();
  e->field_index = bt_str_init();
  scope_top_obj_insert(cxt, OBJ_ENUM, e);
  return e;
}

void enum_free(void *ptr) {
  enum_t *e = (enum_t *)ptr;
  list_free(e->field_list);
  bt_free(e->field_index);
  free(e);
}

// If the decl node does not have a T_BASETYPE node as first child (i.e. first child T_)
// then the additional basetype node may provide the base type; Caller must free memory
// 1. Do not process storage class including typedef - the caller should add them to symbol table
// 2. Do process struct/union/enum definition
// 3. Type is only valid within the scope it is analyzed
type_t *type_gettype(type_cxt_t *cxt, token_t *decl, token_t *basetype, uint32_t flags) {
  assert(basetype->type == T_BASETYPE);
  token_t *op = ast_getchild(decl, 1);
  token_t *decl_name = ast_getchild(decl, 2);
  assert(decl_name->type == T_ || decl_name->type == T_IDENT);
  decl_prop_t basetype_type = BASETYPE_GET(basetype->decl_prop); // The type primitive of base type decl; only valid with BASETYPE_*
  type_t *curr_type = type_init(cxt); // Allocate a new type variable
  curr_type->decl_prop = basetype->decl_prop; // This may copy qualifier and storage class of the base type
  if(!(flags & TYPE_ALLOW_STGCLS) && (basetype->decl_prop & DECL_STGCLS_MASK)) 
    error_row_col_exit(basetype->offset, "Storage class modifier is not allowed in this context\n");
  if(basetype_type == BASETYPE_STRUCT || basetype_type == BASETYPE_UNION) {
    token_t *su = ast_getchild(basetype, 0);
    assert(su && (su->type == T_STRUCT || su->type == T_UNION));
    // If there is no name and no derivation and no body (checked inside type_getcomp) then this is forward
    curr_type->comp = type_getcomp(cxt, su, decl_name->type == T_ && op->type == T_); 
    curr_type->size = curr_type->comp->size; // Could be unknown size
  } else if(basetype_type == BASETYPE_ENUM) {
    // TODO: ADD PROCESSING FOR ENUM
  } else if(basetype_type == BASETYPE_UDEF) {
    token_t *udef_name = ast_getchild(basetype, 0);
    assert(udef_name && udef_name->type == T_IDENT);
    curr_type->udef_type = (type_t *)scope_search(cxt, SCOPE_UDEF, udef_name->str); // May return a struct with or without def
    curr_type->udef_name = udef_name->str;
    assert(curr_type->udef_type); // Must have been defined, otherwise the parser will not recognize it as udef
    curr_type->size = curr_type->udef_type->size;
  } else { // This branch is for primitive base types
    if(basetype_type == BASETYPE_VOID) {
      // const void * is also illegal
      if(DECL_STGCLS_MASK & basetype->decl_prop) { error_row_col_exit(basetype->offset, "\"void\" type cannot have storage class\n"); }
      else if(DECL_QUAL_MASK & basetype->decl_prop) { error_row_col_exit(basetype->offset, "\"void\" type cannot have qualifiers\n"); }
      else if(!(flags & TYPE_ALLOW_VOID) && op->type == T_) { // void base type, and no derivation (we allow void *)
        error_row_col_exit(basetype->offset, "\"void\" type can only be used in function argument and return value\n");
      }
      curr_type->size = 0; // Void type does not have valid size, use 0 as placeholder
    } else if(basetype_type >= BASETYPE_CHAR && basetype_type <= BASETYPE_ULLONG) {
      curr_type->size = ints[BASETYPE_INDEX(basetype->decl_prop)].size;
    } else {
      error_row_col_exit(basetype->offset, "Sorry, type \"%s\" not yet supported\n", token_decl_print(basetype_type));
    }
  }

  while(op->type != T_) {
    assert(op && (op->type == EXP_DEREF || op->type == EXP_FUNC_CALL || op->type == EXP_ARRAY_SUB));
    type_t *parent_type = type_init(cxt);
    parent_type->next = curr_type;
    parent_type->decl_prop = op->decl_prop; // This copies pointer qualifier (const, volatile)
    if(op->type == EXP_DEREF) {
      parent_type->decl_prop |= TYPE_OP_DEREF;
      parent_type->size = TYPE_PTR_SIZE;
    } else if(op->type == EXP_ARRAY_SUB) {
      parent_type->decl_prop |= TYPE_OP_ARRAY_SUB;
      parent_type->array_size = op->array_size;
      // If lower type is unknown size (e.g. another array or struct without definition), or the array size not given 
      // then current size is also unknown
      if(op->array_size == -1 || curr_type->size == TYPE_UNKNOWN_SIZE) parent_type->size = TYPE_UNKNOWN_SIZE;
      else parent_type->size = curr_type->size * (size_t)op->array_size;
    } else if(op->type == EXP_FUNC_CALL) {
      parent_type->decl_prop |= TYPE_OP_FUNC_CALL;
      parent_type->size = TYPE_PTR_SIZE;
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
        arg_type = type_gettype(cxt, arg_decl, arg_basetype, TYPE_ALLOW_VOID); // This allows both base and decl to be void type
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
      f->type = type_gettype(cxt, decl, basetype, 0); // Set field type; do not allow void and storage class
      token_t *field_name = ast_getchild(decl, 2);
      if(field_name->type == T_IDENT) {
        f->name = field_name->str;             // Set field name if there is one
        f->source_offset = field_name->offset; // Set field offset to the name for error reporting
      } else { f->source_offset = field->offset; } // If anonymous field, set offset from the field token
      token_t *bf = ast_getchild(field, 1); // Set bit field (2nd child of T_COMP_FIELD)
      if(bf != NULL) {
        assert(bf->type == T_BITFIELD);
        if(!type_is_integer(f->type)) error_row_col_exit(bf->offset, "Bit field can only be defined with integers\n");
        f->bitfield_size = field->bitfield_size; // Could be -1 if there is no bit field
        if((size_t)f->bitfield_size > f->type->size * 8) 
          error_row_col_exit(field->offset, "Bit field \"%s\" size (%lu bits) must not exceed the integer size (%lu bits)\n", 
            type_printable_name(f->name), (size_t)f->bitfield_size, f->type->size * 8);
      } else { f->bitfield_size = f->bitfield_offset = -1; }
      // TODO: ADD BIT FIELD PADDING AND COALESCE
      f->offset = curr_offset; // Set size and offset (currently no alignment)
      f->size = f->type->size;
      if(f->size == TYPE_UNKNOWN_SIZE) // If there is no name then the T_COMP_FIELD has no offset
        error_row_col_exit(f->name ? f->source_offset : basetype->offset, 
          "Struct or union member \"%s\" size is unknown\n", type_printable_name(f->name));
      
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
        printf("%s %d\n", f->name, f->bitfield_size);
        // Non-Bit field always increments the offset; This works also for promoted types
        // Bit field coalesce rules:
        // (1) Adjacent entries of the same integer type will be coalesced; If they cannot be packed into the 
        //     declared base type, we leave a gap and start a new field;
        // (2) Different types (incl. sign difference) are never coalesced;
        // (3) Packed fields are arranged from lower bits to higher bits; Unused bits will be ignored
        // (4) Coalesced bit fields take the storage identical to the base type
        if(!prev_field) { // First entry in struct
          if(f->bitfield_size != -1) f->bitfield_offset = 0;
          curr_offset += f->type->size;
        } else if(f->bitfield_size == -1 && prev_field->bitfield_size == -1) {
          curr_offset += f->type->size; // Both normal fields; size has been set above
        } else if(f->bitfield_size == -1 && prev_field->bitfield_size != -1) {
          curr_offset += f->type->size;
        } else if(f->bitfield_size != -1 && prev_field->bitfield_size == -1) {
          f->bitfield_offset = 0; // Starting a bit field (may potentially have more later)
          curr_offset += f->type->size;
        } else { // Both are valid bitfields - try to coalesce!
          assert(f->bitfield_size != -1 && prev_field->bitfield_size != -1);
          //printf("%s %s %d %d\n", prev_field->name, f->name, prev_field->bitfield_size, f->bitfield_size);
          // If types differ, start a new bit field
          if(BASETYPE_GET(prev_field->type->decl_prop) != BASETYPE_GET(f->type->decl_prop)) {
            f->bitfield_offset = 0;
            curr_offset += prev_field->type->size;
          } else if(prev_field->bitfield_offset + prev_field->bitfield_size + f->bitfield_size <= (int)prev_field->size * 8) {
            f->offset = prev_field->offset; 
            f->bitfield_offset = prev_field->bitfield_offset + prev_field->bitfield_size;
          } else {
            f->offset = prev_field->offset + prev_field->type->size; 
            f->bitfield_offset = 0;
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

enum_t *type_getenum(type_cxt_t *cxt, token_t *token);

obj_free_func_t obj_free_func_list[OBJ_TYPE_COUNT + 1] = {  // Object free functions
  type_free,
  comp_free,
  field_free,
  enum_free,
  NULL,       // Sentinel - will segment fault
};