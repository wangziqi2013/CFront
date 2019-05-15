
#include "token.h"
#include "type.h"
#include "eval.h"
#include "ast.h"
#include "str.h"

// The size of base types; Right shift 16 bits and index into the table. Only integers are applicable
int type_intsizes[11] = {
  -1,         // Illegal
  1, 2, 4, 8, // char short int long
  1, 2, 4, 8, // uchar ushort uint ulong
  16, 16,     // llong ullong
};

type_t builtin_types[] = {
  {TYPE_INDEX_VOID, BASETYPE_VOID, NULL, NULL, -1},
  {TYPE_INDEX_CHAR, BASETYPE_CHAR, NULL, NULL, 1},
  {TYPE_INDEX_SHORT, BASETYPE_SHORT, NULL, NULL, 2},
  {TYPE_INDEX_INT, BASETYPE_INT, NULL, NULL, 4},
  {TYPE_INDEX_LONG, BASETYPE_LONG, NULL, NULL, 8},
  {TYPE_INDEX_UCHAR, BASETYPE_UCHAR, NULL, NULL, 1},
  {TYPE_INDEX_USHORT, BASETYPE_USHORT, NULL, NULL, 2},
  {TYPE_INDEX_UINT, BASETYPE_UINT, NULL, NULL, 4},
  {TYPE_INDEX_ULONG, BASETYPE_ULONG, NULL, NULL, 8},
};

// Given a decl_prop, return the integer size. The decl prop must be an integer type
int type_getintsize(decl_prop_t decl_prop) {
  assert(BASETYPE_GET(decl_prop) == decl_prop); // Make sure there is no other bits set
  assert(BASETYPE_INDEX(decl_prop) > 0 && BASETYPE_INDEX(decl_prop) < sizeof(type_intsizes) / sizeof(*type_intsizes));
  return type_intsizes[BASETYPE_INDEX(decl_prop)];
}

// Returns a int type object, which is allocated on the heap
type_t *type_getinttype() {
  return NULL;
}

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
  cxt->types = vector_init();
  int type_count = (int)sizeof(builtin_types) / (int)sizeof(type_t);
  for(int i = 0;i < type_count;i++) vector_append(cxt->types, NULL); // Make space and let size grow
  for(int i = 0;i < type_count;i++) {
    type_t *type = (type_t *)malloc(sizeof(type_t));
    SYSEXPECT(type != NULL);
    memcpy(type, builtin_types + i, sizeof(type_t));
    *vector_addrat(cxt->types, type->typeid) = type;
  }
  return cxt;
}

void type_free(type_cxt_t *cxt) {
  while(scope_numlevel(cxt)) scope_decurse(cxt); // First pop all scopes
  stack_free(cxt->scopes);
  for(int i = 0;i < vector_size(cxt->types);i++) free(vector_at(cxt->types, i));
  vector_free(cxt->types);
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

// If the decl node does not have a T_BASETYPE node as first child (i.e. first child NULL)
// then the additional basetype node may provide the base type; Caller must free memory
type_t *type_gettype(token_t *decl, token_t *basetype) {
  return NULL;
}

// Input must be T_STRUCT or T_UNION; Caller must free memory
comp_t *type_getcomp(token_t *token) {
  assert(token->type == T_STRUCT || token->type == T_UNION);
  comp_t *comp = (comp_t *)malloc(sizeof(comp_t));
  SYSEXPECT(comp != NULL);
  memset(comp, 0x00, sizeof(comp_t));
  comp->fields = list_str_init();
  comp->index = ht_str_init();
  token_t *name = ast_getchild(token, 0);
  if(name->type == T_IDENT) comp->name = name->str;
  token_t *entry = ast_getchild(token, 1);
  return NULL;
}

void type_freecomp(comp_t *comp) {

}