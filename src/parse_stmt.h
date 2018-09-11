
#ifndef _PARSE_STMT_H
#define _PARSE_STMT_H

#include "parse_exp.h"

typedef parse_exp_cxt_t parse_stmt_cxt_t;

parse_stmt_cxt_t *parse_stmt_init(char *input);
void parse_stmt_free(parse_stmt_cxt_t *cxt);

#endif