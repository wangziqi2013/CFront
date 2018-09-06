
#ifndef _PARSE_EXP_H
#define _PARSE_EXP_H

#include "stack.h"
#include "token.h"
#include "ast.h"
#include "hashtable.h"

#define AST_STACK 0
#define OP_STACK 1

typedef struct {
  // Either AST_STACK or OP_STACK; do not need save because a shift will happen
  int last_active_stack;
  stack_t *stacks[2];
  stack_t *tops[2];
  token_cxt_t *token_cxt;
} parse_exp_cxt_t;

parse_exp_cxt_t *parse_exp_init(char *input);
void parse_exp_free(parse_exp_cxt_t *cxt);
int parse_exp_outermost(parse_exp_cxt_t *cxt);
int parse_exp_hasmatch(parse_exp_cxt_t *cxt, token_t *token);
int parse_exp_isexp(parse_exp_cxt_t *cxt, token_t *token);
int parse_exp_isprimary(parse_exp_cxt_t *cxt, token_t *token);
int parse_exp_la_isdecl(parse_exp_cxt_t *cxt);
int parse_exp_size(parse_exp_cxt_t *cxt, int stack_id);
token_t *parse_exp_peek(parse_exp_cxt_t *cxt, int stack_id);
token_t *parse_exp_peek_at(parse_exp_cxt_t *cxt, int stack_id, int index);
int parse_exp_isempty(parse_exp_cxt_t *cxt, int stack_id);
void parse_exp_recurse(parse_exp_cxt_t *cxt);
void parse_exp_decurse(parse_exp_cxt_t *cxt);
token_t *parse_exp_next_token(parse_exp_cxt_t *cxt);
void parse_exp_shift(parse_exp_cxt_t *cxt, int stack_id, token_t *token);
token_t *parse_exp_reduce(parse_exp_cxt_t *cxt, int op_num_override, int allow_paren);
void parse_exp_reduce_preced(parse_exp_cxt_t *cxt, token_t *token);
token_t *parse_exp_reduce_all(parse_exp_cxt_t *cxt);
token_t *parse_exp(parse_exp_cxt_t *cxt);

#endif