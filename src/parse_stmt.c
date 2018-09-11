
#include "parse_stmt.h"

parse_stmt_cxt_t *parse_stmt_init(char *input) { return parse_exp_init(input); }
void parse_stmt_free(parse_stmt_cxt_t *cxt) { parse_exp_free(cxt); }

token_t *parse_stmt(parse_stmt_cxt_t *cxt) {
  (void)cxt; return NULL;
}