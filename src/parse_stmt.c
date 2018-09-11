
#include "parse_stmt.h"

parse_stmt_cxt_t *parse_stmt_init(char *input) { return parse_exp_init(input); }
void parse_stmt_free(parse_stmt_cxt_t *cxt) { parse_exp_free(cxt); }

// Return a labeled statement
token_t *parse_labeled_stmt(parse_stmt_cxt_t *cxt, token_type_t type) {
  (void)cxt; (void)type; return NULL;
}

// Returns an expression statement
token_t *parse_exp_stmt(parse_stmt_cxt_t *cxt) {
  (void)cxt; return NULL;
}

token_t *parse_stmt(parse_stmt_cxt_t *cxt) {
  token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
  switch(la->type) {
    case T_DEFAULT: // Fall through
    case T_CASE: return parse_labeled_stmt(cxt, la->type);
    case T_IDENT: 
      if(token_lookahead_notnull(cxt->token_cxt, 2)->type == T_COLON) return parse_labeled_stmt(cxt, la->type);
      else return parse_exp_stmt(cxt, la->type);
    case T_LCPAREN: return parse_comp_stmt(cxt);
  }
  (void)cxt; return NULL;
}