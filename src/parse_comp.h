
#ifndef _PARSE_COMP_H
#define _PARSE_COMP_H

#include "parse_decl.h"

typedef parse_exp_cxt_t parse_comp_cxt_t;

parse_decl_cxt_t *parse_comp_init(char *input);
void parse_comp_free(parse_decl_cxt_t *cxt);

#endif