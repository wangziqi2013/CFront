
#ifndef _AST_H
#define _AST_H

#include "token.h"

#define AST_MAX_CHILD 8

typedef struct ast_t {
  token_t *token;
  struct ast_t *child[AST_MAX_CHILD];
} ast_t;

#endif