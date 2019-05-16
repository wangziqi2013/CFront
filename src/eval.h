
#ifndef _EVAL_H
#define _EVAL_H

#include "token.h"
#include "ast.h"

struct value_t_struct;
typedef struct value_t_struct value_t;

// Evaluating const expression using native int types
int eval_const_int_getimm(token_t *token); // Works for integer immediate values, signed integer only; error if type incorrect
int eval_const_int(token_t *token); // Integer expression, no type info, only used for array range expression

// Converts a integer literal token into binary value form
void eval_getintimm(value_t *val, token_t *token);
value_t *eval_constexpr(token_t *token);

#endif