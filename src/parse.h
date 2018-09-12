
#include "parse_exp.h"
#include "parse_decl.h"
#include "parse_comp.h"
#include "parse_stmt.h"

#ifndef _PARSE_H
#define _PARSE_H

typedef parse_exp_cxt_t parse_cxt_t;

parse_stmt_cxt_t *parse_init(char *input);
void parse_free(parse_cxt_t *cxt);
token_t *parse(parse_cxt_t *cxt);

#endif