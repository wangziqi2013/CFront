
#ifndef _EVAL_H
#define _EVAL_H

#include "token.h"
#include "ast.h"
#include "type.h"

#define EVAL_MAX(a, b) (a > b ? a : b)
#define EVAL_MIN(a, b) (a < b ? a : b)

// Used as the parameter to eval_const_atoi()
#define ATOI_NO_CHECK_END 0
#define ATOI_CHECK_END    1


// Result type of two integers in an expression
decl_prop_t eval_int_convert(decl_prop_t int1, decl_prop_t int2);

// Take a maximum bite and return the next to read
char *eval_const_atoi_maxbite(char *s, int base, token_t *token, int *ret); 
// Given a string and base convert to integer
int eval_const_atoi(char *s, int base, token_t *token, int max_char, int check_end, char **next); 
char eval_escaped_char(char escaped, token_t *token);

char eval_const_char_token(token_t *token); // Evaluates char type token to char
str_t *eval_const_str_token(token_t *token) // Evaluates string token to str_t *
// Evaluating const expression using native int types (or convert other types to int)
int eval_const_int_token(token_t *token); // Evaluates an integer type or char type token to int
int eval_const_int(type_cxt_t *cxt, token_t *token); // Integer expression, no type info, only used for array range expression

#endif