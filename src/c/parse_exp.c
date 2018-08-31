

#include "parse_exp.h"

parse_exp_context_t *parse_exp_init() {
  parse_exp_context_t *cxt = (parse_exp_context_t *)malloc(sizeof(parse_exp_context_t));
  if(cxt == NULL) perror(__func__);
  cxt->stacks[0] = stack_init();
  cxt->stacks[1] = stack_init();
  // If the first token is an operator then it must be prefix operator
  cxt->last_active_stack = 1;
  return cxt;
}

void parse_exp_free(parse_exp_context_t *cxt) {
  stack_free(cxt->stacks[0]);
  stack_free(cxt->stacks[1]);
  free(cxt);
  return;
}