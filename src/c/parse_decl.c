
#include "parse_decl.h"

parse_decl_cxt_t *parse_decl_init(char *input) {
  return parse_exp_init(input);
}

void parse_decl_free(parse_decl_cxt_t *cxt) {
  parse_exp_free(cxt);
}

// Whether the next token is decl. Note that this is context dependent, and thus 
// makes C syntex not context-free. Need to check typedef table
// Note: The following tokens are considered as part of a type expression:
//   1. ( ) [ ] *  2. specifiers, qualifiers and types 3. typedef'ed names
// struct, union and enum are recognized here, but they need another parser
int parse_decl_isdecl(parse_decl_cxt_t *cxt, token_t *token) {
  token_type_t type = token->type;
  stack_t *op = cxt->stacks[OP_STACK];
  if(!stack_empty(op) && ((token_t *)stack_peek(op))->type == T_STOP && 
     (type == T_RPAREN || type == T_RSPAREN)) return 0; 
  else if(kwd_istype(token->type)) return 1;
  else if(token->type == T_IDENT && ht_find(cxt->udef_types, token->str) != HT_NOTFOUND) return 1;
  switch(token->type) {
    case T_LPAREN: case T_RPAREN: case T_STAR: case T_LSPAREN: case T_RSPAREN: return 1;
  }
  return 0;
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
  switch(token->type) {
    case T_LPAREN: token->type = EXP_FUNC_CALL; break;
    case T_RPAREN: token->type = EXP_RPAREN; break;
    case T_STAR: token->type = EXP_DEREF; break;
    case T_LSPAREN: token->type = EXP_ARRAY_SUB; break;
    case T_RSPAREN: token->type = EXP_RSPAREN; break;
  }
  return token;
}