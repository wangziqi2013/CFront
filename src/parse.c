
#include "parse.h"

parse_stmt_cxt_t *parse_init(char *input) { return parse_exp_init(input); }
void parse_free(parse_cxt_t *cxt) { parse_exp_free(cxt); }