
#include "parse_comp.h"

parse_decl_cxt_t *parse_comp_init(char *input) { return parse_exp_init(input); }
void parse_comp_free(parse_comp_cxt_t *cxt) { parse_exp_free(cxt); }