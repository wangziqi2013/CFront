
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

void ast_print(token_t *token, int depth) {
  for(int i = 0;i < depth;i++) putchar('  ');
  const char *symstr = token_symstr(token->type);
  printf("%s %s", token_typestr(token->type), symstr == NULL ? "" : symstr);
  for(token_t *child = token->child;child != NULL; child = child->sibling) 
    ast_pre_traverse(child, depth + 1);
  return;
}