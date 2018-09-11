
#ifndef _PARSE_STMT_H
#define _PARSE_STMT_H

#include "parse_exp.h"

typedef parse_exp_cxt_t parse_stmt_cxt_t;

parse_stmt_cxt_t *parse_stmt_init(char *input);
void parse_stmt_free(parse_stmt_cxt_t *cxt);
token_t *parse_lbl_stmt(parse_stmt_cxt_t *cxt, token_type_t type);
token_t *parse_comp_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_if_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_switch_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_while_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_do_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_for_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_goto_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_brk_cont_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_return_stmt(parse_stmt_cxt_t *cxt);
token_t *parse_init_list(parse_stmt_cxt_t *cxt);
token_t *parse_stmt(parse_stmt_cxt_t *cxt);

#endif