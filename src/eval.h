
#ifndef _EVAL_H
#define _EVAL_H

#include "token.h"

struct value_t_struct;
typedef struct value_t_struct value_t;
// Converts a integer literal token into binary value form
void eval_getintimm(value_t *val, token_t *token);
value_t *eval_constexpr(token_t *token);

#endif