
#ifndef _PARSE_DECL_H
#define _PARSE_DECL_H

#include "parse_exp.h"
#include "hashtable.h"

typedef parse_exp_cxt_t parse_decl_cxt_t;

parse_decl_cxt_t *parse_decl_init(char *input);
void parse_decl_free(parse_decl_cxt_t *cxt);
int parse_decl_isbasetype(parse_decl_cxt_t *cxt, token_t *token);
token_t *parse_decl_next_token(parse_decl_cxt_t *cxt);
void parse_basetype(parse_decl_cxt_t *cxt, token_t *basetype);
token_t *parse_decl(parse_decl_cxt_t *cxt);

#endif