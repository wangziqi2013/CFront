
#include "parse_comp.h"

parse_decl_cxt_t *parse_comp_init(char *input) { return parse_exp_init(input); }
void parse_comp_free(parse_comp_cxt_t *cxt) { parse_exp_free(cxt); }

// This parses struct or union or enum
token_t *parse_comp(parse_exp_cxt_t *cxt) {
  token_t *token = token_get_next(cxt->token_cxt);
  assert(token);
  switch(token->type) {
    case T_STRUCT: case T_UNION: return parse_struct_union(cxt, token);
    case T_ENUM: return parse_enum(cxt, token);
    default: assert(0);
  }
}