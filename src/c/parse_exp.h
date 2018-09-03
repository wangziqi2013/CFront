
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
int parse_exp_isexp(parse_exp_cxt_t *cxt, token_t *token);
int parse_exp_isprimary(parse_exp_cxt_t *cxt, token_t *token);
int parse_exp_istype(parse_exp_cxt_t *cxt);
void parse_exp_free_token(token_t *token);
token_t *parse_exp_next_token(parse_exp_cxt_t *cxt);
void parse_exp_shift(parse_exp_cxt_t *cxt, int stack_id, token_t *token);
token_t *parse_exp_reduce(parse_exp_cxt_t *cxt, int op_num_override);
void parse_exp_reduce_preced(parse_exp_cxt_t *cxt, token_t *token);
token_t *parse_exp_reduce_all(parse_exp_cxt_t *cxt);
token_t *parse_exp(parse_exp_cxt_t *cxt);

#endif