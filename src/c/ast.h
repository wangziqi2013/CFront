
#ifndef _AST_H
#define _AST_H

#include "token.h"

token_t *ast_make_node(token_t *token);
int ast_isleaf(token_t *token);
token_t *ast_append_child(token_t *token, token_t *child);
token_t *ast_push_child(token_t *token, token_t *child);
void ast_print(token_t *token, int depth);

#endif