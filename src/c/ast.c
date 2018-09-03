
#include "ast.h"

// Initialize a token to be an AST node. Return the node given to it
token_t *ast_make_node(token_t *token) {
  token->child = token->sibling = NULL;
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
  return token;
}

// Adds the node as the first child of the token
token_t *ast_push_child(token_t *token, token_t *child) {
  child->sibling = token->child;
  token->child = child;
  return token;
}

// Adds a node as a sibling after the given one, adding a child
token_t *ast_insert_after(token_t *token, token_t *child) {
  child->sibling = token->sibling;
  token->sibling = child;
  return token;
}

void ast_print(token_t *token, int depth) {
  for(int i = 0;i < depth * 2;i++) putchar(' ');
  const char *symstr = token_symstr(token->type);
  printf("%d:%s %s\n", token->type, token_typestr(token->type), 
         symstr == NULL ? (token->type >= T_LITERALS_BEGIN && token->type < T_LITERALS_END ? token->str : "") : symstr);
  for(token_t *child = token->child;child != NULL; child = child->sibling) 
    ast_print(child, depth + 1);
  return;
}

void _ast_collect_funcarg(token_t *comma, token_t *token) {
  assert(comma->child != NULL && comma->child->sibling != NULL);
  if(comma->child->type != EXP_COMMA) {
    ast_append_child(token, comma->child);
    ast_append_child(token, comma->child->sibling);
  } else {
    _ast_collect_funcarg(comma->child, token);
    ast_append_child(token, comma->child->sibling);
  }
  // We know it is comma node, so could just free
  free(comma);
  return;
}

// Transforms function argument from comma expression to flat structure
// Three cases: argument less func; one argument func (must not be comma exp)
// and functions with >= 2 arguments
void ast_collect_funcarg(token_t *token) {
  assert(token->type == EXP_FUNC_CALL);
  token_t *comma = token->child->sibling;
  if(comma == NULL || comma->type != EXP_COMMA) return;
  _ast_collect_funcarg(comma, token);
  return;
}