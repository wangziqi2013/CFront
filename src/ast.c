
#include "ast.h"

// Initialize a token to be an AST node. Return the node given to it
token_t *ast_make_node(token_t *token) {
  token->child = token->sibling = token->parent = NULL;
  return token;
}

int ast_isleaf(token_t *token) { return token->child == NULL; }

token_t *ast_append_child(token_t *token, token_t *child) {
  if(token->child == NULL) token->child = child;
  else {
    token_t *last = token->child;
    while(last->sibling != NULL) last = last->sibling;
    last->sibling = child;
  }
  child->sibling = NULL;
  child->parent = token;
  return token;
}

// Adds the node as the first child of the token
token_t *ast_push_child(token_t *token, token_t *child) {
  child->sibling = token->child;
  token->child = child;
  child->parent = token;
  return token;
}

// Adds a node as a sibling after the given one, adding a child
token_t *ast_insert_after(token_t *token, token_t *child) {
  child->sibling = token->sibling;
  token->sibling = child;
  child->parent = token->parent;
  return token;
}

// Remove from parent node. Assume there is a parent node. Returns the node itself
token_t *ast_remove(token_t *token) {
  token_t *parent = token->parent;
  if(parent->child == token) parent->child = token->sibling;
  else {
    token_t *curr = parent->child; // Assumes that the tree is correctly formed, so curr will not be NULL
    while(curr->sibling != token) curr = curr->sibling; 
    curr->sibling = token->sibling;
  }
  return token;
}

void ast_print(token_t *token, int depth) {
  for(int i = 0;i < depth * 2;i++) if(i % 2 == 0) printf("|"); else printf(" ");
  const char *symstr = token_symstr(token->type);
  char array_size[16];
  if(token->type == EXP_ARRAY_SUB) sprintf(array_size, "%d", token->array_size);
  printf("%04d:%s %s%s\n", token->type, token_typestr(token->type), 
         token->type == T_BASETYPE ? token_decl_print(token->decl_prop) : 
         (symstr == NULL ? (token->type >= T_LITERALS_BEGIN && token->type < T_LITERALS_END ? token->str : "") : symstr),
         token->type == EXP_ARRAY_SUB ? array_size : "");
  for(token_t *child = token->child;child != NULL; child = child->sibling) ast_print(child, depth + 1);
  return;
}

// Releases memory for every node in the AST
void ast_free(token_t *token) {
  while(token->child != NULL) {
    token_t *next = token->child->sibling;
    ast_free(token->child);
    token->child = next;
  }
  token_free(token);
}

// Get n-th child; Return NULL if index is larger than the number of children
token_t *ast_getchild(token_t *token, int index) {
  assert(index >= 0 && token != NULL);
  token = token->child;
  while(token != NULL && index-- != 0) token = token->sibling;
  return token;
}

// Returns the last inserted node
token_t *_ast_collect_funcarg(token_t *comma, token_t *token) {
  assert(ast_getchild(comma, 0) != NULL && ast_getchild(comma, 1) != NULL);
  token_t *child1 = comma->child, *child2 = child1->sibling;
  if(child1->type != EXP_COMMA) {
    ast_insert_after(token, child2);
    ast_insert_after(token, child1);
    token = child1;
  } else {
    ast_insert_after(token, child2);
    token = _ast_collect_funcarg(child1, token);
  }
  token_free(comma);
  return token;
}

// Transforms function argument from comma expression to flat structure
// Three cases: argument-less func; one argument func (must not be comma exp)
// and functions with >= 2 arguments
void ast_collect_funcarg(token_t *token) {
  assert(token->type == EXP_FUNC_CALL);
  token_t *comma = ast_getchild(token, 1);
  if(comma == NULL || comma->type != EXP_COMMA) return;
  // The comma node has been freed. The function returns the last node inserted
  token->child->sibling = _ast_collect_funcarg(comma, comma);
  return;
}

// Transforms conditional expression from two 2-operand operators to
// a signle cond operator
void ast_movecond(token_t *token) {
  assert(token->type == EXP_COND);
  if(ast_getchild(token, 1)->type != EXP_COLON) 
    error_row_col_exit(token->offset, "Operator \'?\' must be followed by operator \':\'\n");
  token_t *colon = ast_getchild(token, 1), *child2 = ast_getchild(colon, 1);
  ast_append_child(token, colon->child);
  ast_append_child(token, child2);
  token->child->sibling = colon->child;
  token_free(colon);
  return;
}

// Returns a pointer to the first child of given type, or NULL
token_t *ast_gettype(token_t *token, token_type_t type) {
  for(token = token->child;token && token->type != type;token = token->sibling);
  return token;
}
