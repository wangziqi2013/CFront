
#ifndef _PARSE_EXP_H
#define _PARSE_EXP_H

#include "stack.h"
#include "token.h"
#include "ast.h"

typedef struct {
  // Either 0 or 1
  int last_active_stack;
  stack_t *stacks[2];
} parse_exp_cxt_t;

parse_exp_cxt_t *parse_exp_init();
void parse_exp_free(parse_exp_cxt_t *cxt);

#endif