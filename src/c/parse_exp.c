
#include "parse_exp.h"

parse_exp_cxt_t *parse_exp_init(char *input) {
  parse_exp_cxt_t *cxt = (parse_exp_cxt_t *)malloc(sizeof(parse_exp_cxt_t));
  if(cxt == NULL) perror(__func__);
  cxt->stacks[0] = stack_init();
  cxt->stacks[1] = stack_init();
  // If the first token is an operator then it must be prefix operator
  cxt->last_active_stack = OP_STACK;
  cxt->s = input;
  cxt->allow_comma = 1;
  return cxt;
}

void parse_exp_free(parse_exp_cxt_t *cxt) {
  stack_free(cxt->stacks[0]);
  stack_free(cxt->stacks[1]);
  free(cxt);
  return;
}

// Determine if a token could continue an expression currently being parsed
// Literals (incl. ident), operators, and sizeof() could be part of an expression
int parse_exp_isexp(parse_exp_cxt_t *cxt, token_t *token) {
  token_type_t type = token->type;
  if(cxt->allow_comma == 0 && type == T_COMMA) return 0; 
  return ((type >= T_OP_BEGIN && type < T_OP_END) || 
          (type >= T_LITERALS_BEGIN && type < T_LITERALS_END) || 
          (type == T_SIZEOF));
}

void parse_exp_next_token(parse_exp_cxt_t *cxt) {
  token_t *token = (token_t *)malloc(sizeof(token_t));
  cxt->s = token_get_next(cxt->s, token);
  if(cxt->s == NULL || !parse_exp_isexp(cxt, token)) {
    free(token);
    // TODO: FINISHED ALL TOKENS
    // TODO: Maybe check for type cast?
  }

  // If the last active stack is AST stack, then the op must be postfix (unary)
  // or binary
  if(cxt->last_active_stack == AST_STACK) {

  } else {

  }
}
