
#include "ast.h"

// Initialize a token to be an AST node. Return the node given to it
token_t *ast_make_node(token_t *token) {
  token->sibling = NULL;
  return token;
}