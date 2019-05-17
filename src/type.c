
#include "token.h"
#include "type.h"
#include "eval.h"
#include "ast.h"
#include "str.h"

scope_t *scope_init(int level) {
  scope_t *scope = (scope_t *)malloc(sizeof(scope_t));
  SYSEXPECT(scope != NULL);
  scope->level = level;
  for(int i = 0;i < SCOPE_TYPE_COUNT;i++) scope->names[i] = ht_str_init();
  return scope;
}

void scope_free(scope_t *scope) {
  for(int i = 0;i < SCOPE_TYPE_COUNT;i++) ht_free(scope->names[i]);
  free(scope);
  return;
}

type_cxt_t *type_init() {
  type_cxt_t *cxt = (type_cxt_t *)malloc(sizeof(type_cxt_t));
  SYSEXPECT(cxt != NULL);
  cxt->scopes = stack_init();
  scope_recurse(cxt);
  return cxt;
}

void type_free(type_cxt_t *cxt) {
  while(scope_numlevel(cxt)) scope_decurse(cxt); // First pop all scopes
  stack_free(cxt->scopes);
  free(cxt);
}

hashtable_t *scope_atlevel(type_cxt_t *cxt, int level, int type) {
  return ((scope_t *)stack_peek_at(cxt->scopes, stack_size(cxt->scopes) - 1 - level))->names[type];
}
hashtable_t *scope_top(type_cxt_t *cxt, int type) { return ((scope_t *)stack_peek_at(cxt->scopes, 0))->names[type]; }
int scope_numlevel(type_cxt_t *cxt) { return stack_size(cxt->scopes); }
void scope_recurse(type_cxt_t *cxt) { stack_push(cxt->scopes, scope_init(scope_numlevel(cxt))); }
void scope_decurse(type_cxt_t *cxt) { scope_free(stack_pop(cxt->scopes)); }
void *scope_top_find(type_cxt_t *cxt, int type, void *key) { return ht_find(scope_top(cxt, type), key); }
void *scope_top_insert(type_cxt_t *cxt, int type, void *key, void *value) { return ht_insert(scope_top(cxt, type), key, value); }

// Searches all levels of scopes and return the first one; return NULL if not found
void *scope_search(type_cxt_t *cxt, int type, void *name) {
  assert(type >=0 && type < SCOPE_TYPE_COUNT && scope_numlevel(cxt) > 0);
  for(int level = scope_numlevel(cxt) - 1;level >= 0;level--) {
    void *value = ht_find(scope_atlevel(cxt, level, type), name);
    if(value != HT_NOTFOUND) return value;
  }
  return NULL;
}

// If the decl node does not have a T_BASETYPE node as first child (i.e. first child T_)
// then the additional basetype node may provide the base type; Caller must free memory
type_t *type_gettype(type_cxt_t *cxt, token_t *decl, token_t *basetype) {
  type_t *type = (type_t *)malloc(sizeof(type_t));
  SYSEXPECT(type != NULL);
  memset(type, 0x00, sizeof(type_t));
  type->decl_prop = basetype->decl_prop; // This may copy qualifier and storage class of the base type
  decl_prop_t basetype_type = BASETYPE_GET(basetype->decl_prop);
  if(basetype_type == BASETYPE_STRUCT || basetype_type == BASETYPE_UNION) {
    token_t *su = ast_getchild(basetype, 0); // May access the symbol table
    assert(su && (su->type == T_STRUCT || su->type == T_UNION));
    type->comp = type_getcomp(cxt, su);
  } else if(basetype_type == BASETYPE_ENUM) {
    // TODO: ADD PROCESSING FOR ENUM
  } else if(basetype_type == BASETYPE_UDEF) {
    // TODO: PROCESS TYPEDEF BY LOOKING UP SYMBOL TABLE
  }

  token_t stack[TYPE_MAX_DERIVATION]; // Use stack to reverse the derivation chain
  int num_op = 0;
  token_t *op = ast_getchild(decl, 1);
  while(op->type != T_) {
    assert(op->type == EXP_DEREF || op->type == EXP_FUNC_CALL || op->type == EXP_ARRAY_SUB);
    if(num_op == TYPE_MAX_DERIVATION) // Report error if the stack overflows
      error_row_col_exit(op->offset, "Type derivation exceeds maximum allowed (%d)\n", TYPE_MAX_DERIVATION);
    stack[num_op++] = op;
    op = ast_getchild(op, 0); 
    assert(op != NULL);
  }

  type_t *curr_type = type; // Points to the base type at the beginning
  while(num_op > 0) {
    op = stack[--num_op];
    type_t *parent_type = (type_t *)malloc(sizeof(type_t));
    SYSEXPECT(parent_type != NULL);
    memset(parent_type, 0x00, sizeof(type_t));
    parent_type->next = curr_type;
    parent_type = decl_prop = op->decl_prop; // This copies pointer qualifier (const, volatile)
    if(op->type = EXP_DEREF) {
      parent_type->decl_prop |= TYPE_OP_DEREF;
      parent_type->size = TYPE_PTR_SIZE;
    } else if(op->type == EXP_ARRAY_SUB) {
      parent_type->decl_prop |= TYPE_OP_ARRAY_SUB;
      parent_type->array_size = op->array_size;
      // If lower type is unknown size, or the array size not given then current size is also unknown
      if(op->array_size == -1 || curr_type->size == TYPE_UNKNOWN_SIZE) parent_type->size = TYPE_UNKNOWN_SIZE;
      else parent_type->size = curr_type->size * (size_t)op->array_size;
    } else if(op->type == EXP_FUNC_CALL) {
      parent_type->decl_prop |= TYPE_OP_FUNC_CALL;
      parent_type->size = TYPE_PTR_SIZE;
      parent_type->arg_list = list_str_init();
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
        }
        token_t *arg_basetype = ast_getchild(arg_decl, 0);
        token_t *arg_exp = ast_getchild(arg_decl, 1);
        token_t *arg_name = ast_getchild(arg_decl, 2);
        // Detect whether the type is void, and that it is not (the first AND the last)
        if(BASETYPE_GET(arg_basetype->decl_prop) == BASETYPE_VOID && arg_exp->type == T_ && \
           (arg_num > 1 || arg_decl->sibling)) 
           error_row_col_exit(op->offset, "\"void\" must be the first and only argument\n");
        arg_type = type_gettype(cxt, arg_decl, arg_basetype);
        arg_decl = arg_decl->sibling;
      }
    }
    curr_type = parent_type;
  }

  return type;
}

// Input must be T_STRUCT or T_UNION
// This function may add new symbol to the current scope
// TODO: PROCESS FORWARD DECL
comp_t *type_getcomp(type_cxt_t *cxt, token_t *token) {
  assert(token->type == T_STRUCT || token->type == T_UNION);
  token_t *name = ast_getchild(token, 0);
  token_t *entry = ast_getchild(token, 1);
  assert(name && entry); // Both must be there
  int has_name = name->type != T_;
  int has_body = entry->type != T_;
  assert(has_name || has_body); 
  int domain = (token->type == T_STRUCT) ? SCOPE_STRUCT : SCOPE_UNION;
  if(has_name && !has_body) { // There is no body but a name - must be referencing an already defined struct or union
    comp_t *comp = (comp_t *)scope_search(cxt, domain, name->str);
    if(comp == HT_NOTFOUND) error_row_col_exit(token->offset, "Struct or union not yet defined: %s\n", name->str);
    return comp;
  }
  comp_t *comp = (comp_t *)malloc(sizeof(comp_t));
  SYSEXPECT(comp != NULL);
  memset(comp, 0x00, sizeof(comp_t));
  if(has_name && has_body) { // Add the comp to the current scope if there is both name and body
    if(scope_top_find(cxt, domain, name->str) != HT_NOTFOUND) // Check name conflict
      error_row_col_exit(token->offset, "Redefinition of struct of union: %s\n", name->str);
    scope_top_insert(cxt, domain, name->str, comp); // Insert here before processing fields s.t. we can include pointer to itself
  }
  comp->field_list = list_str_init();
  comp->field_index = bt_str_init();
  if(name->type == T_IDENT) comp->name = name->str;
  int curr_offset = 0;
  while(entry) {
    assert(entry->type == T_COMP_DECL);
    token_t *basetype = ast_getchild(entry, 0); // This will be repeatedly used
    assert(basetype->type == T_BASETYPE);
    token_t *field = ast_getchild(entry, 1);
    while(field) {
      assert(field->type == T_COMP_FIELD);
      token_t *decl = ast_getchild(field, 0);
      assert(decl->type == T_DECL);
      field_t *f = (field_t *)malloc(sizeof(field_t));
      SYSEXPECT(f != NULL);
      memset(f, 0x00, sizeof(field_t));
      f->type = type_gettype(cxt, decl, basetype); // Set field type
      token_t *field_name = ast_getchild(decl, 2);
      if(field_name->type == T_IDENT) f->name = field_name->str; // Set field name if there is one
      token_t *bf = ast_getchild(field, 1); // Set bit field (2nd child of T_COMP_FIELD)
      if(bf != NULL) {
        assert(bf->type == T_BITFIELD);
        f->bitfield_size = field->bitfield_size; // Could be -1 if there is no bit field
      }
      // TODO: ADD BIT FIELD PADDING AND COALESCE
      // TODO: ALLOW ANONYMOUS STRUCT/UNION TO BE PROMOTED TO PARENT LEVEL
      f->offset = curr_offset; // Set size and offset (currently no alignment)
      f->size = f->type->size;
      curr_offset += f->type->size;
      if(f->name) { // Only insert if there is a name
        field_t *ret = bt_insert(comp->field_index, f->name, f); // Returns prev element if key exists
        if(ret != f) error_row_col_exit(field_name->offset, 
            "Duplicated field name \"%s\" in composite type declaration\n", f->name);
      }
      list_insert(comp->field_list, f->name, f); // Always insert into the ordered list
    }
    entry = entry->sibling;
  }
  comp->size = curr_offset;
  return comp;
}

void type_freecomp(comp_t *comp) {
  // TODO: FREE TYPE LIST ALSO
  list_free(comp->field_list);
  bt_free(comp->field_index);
}