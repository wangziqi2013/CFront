
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

// Releases memory for every node in the AST
void ast_free(token_t *token) {
  token_t *next;
  while(token->child != NULL) {
    next = token->child->sibling;
    ast_free(token->child);
    token->child = next;
  }
  token_free(token);
}

// Returns the last inserted node
token_t *_ast_collect_funcarg(token_t *comma, token_t *token) {
  assert(comma->child != NULL && comma->child->sibling != NULL);
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
// Three cases: argument less func; one argument func (must not be comma exp)
// and functions with >= 2 arguments
void ast_collect_funcarg(token_t *token) {
  assert(token->type == EXP_FUNC_CALL);
  token_t *comma = token->child->sibling;
  if(comma == NULL || comma->type != EXP_COMMA) return;
  // The comma node has been freed. The function returns the last node inserted
  token->child->sibling = _ast_collect_funcarg(comma, comma);
  return;
}

// Transforms conditional expression from two 2-operand operators to
// a signle cond operator
void ast_movecond(token_t *token) {
  assert(token->type == EXP_COND && token->child->sibling->type == EXP_COLON);
  token_t *colon = token->child->sibling, *child2 = colon->child->sibling;
  ast_append_child(token, colon->child);
  ast_append_child(token, child2);
  token->child->sibling = colon->child;
  token_free(colon);
  return;
}