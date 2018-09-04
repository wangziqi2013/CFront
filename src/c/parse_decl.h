
#ifndef _PARSE_DECL
#define _PARSE_DECL

#include "hashtable.h"

typedef parse_exp_cxt_t parse_decl_cxt_t;

parse_decl_cxt_t *parse_decl_init(char *input);
void parse_decl_free(parse_decl_cxt_t *cxt);
int parse_decl_istype(parse_decl_cxt_t *cxt, token_t *token);
int parse_decl_isdecl(parse_decl_cxt_t *cxt, token_t *token);
token_t *parse_decl_next_token(parse_decl_cxt_t *cxt);
token_t *parse_decl(parse_decl_cxt_t *cxt);

#endif