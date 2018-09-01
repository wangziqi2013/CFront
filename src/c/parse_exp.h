
#ifndef _PARSE_EXP_H
#define _PARSE_EXP_H

#include "stack.h"
#include "token.h"
#include "ast.h"

#define AST_STACK 0
#define OP_STACK 1

typedef struct {
  // Either AST_STACK or OP_STACK
  int last_active_stack;
  stack_t *stacks[2];
  char *s;
  // In function argument lists comma should not be parsed as expression
  int allow_comma;
} parse_exp_cxt_t;

parse_exp_cxt_t *parse_exp_init(char *input);
void parse_exp_free(parse_exp_cxt_t *cxt);

#endif