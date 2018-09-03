
#include "parse_decl.h"

parse_decl_cxt_t *parse_decl_init(char *input) {
  
  return parse_exp_init(input);
}

void parse_decl_free(parse_decl_cxt_t *cxt) {
  
  parse_exp_free(cxt);
}

// Whether the next token is decl. Note that this is context dependent, and thus 
// makes C syntex not context-free. Need to check typedef table
int parse_decl_isdecl(parse_decl_cxt_t *cxt, token_t *token) {
  switch(token->type) {

  }
}

// Same rule as parse_exp_next_token()
token_t *parse_decl_next_token(parse_decl_cxt_t *cxt) {
  token_t *token = token_alloc();
  char *before = cxt->s;
  cxt->s = token_get_next(cxt->s, token);
  if(cxt->s == NULL || !parse_decl_isdecl(cxt, token)) {
    cxt->s = before;
    token_free(token);
    return NULL;
  }
}