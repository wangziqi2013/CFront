
#ifndef _EVAL_H
#define _EVAL_H

#include "token.h"

struct value_t;
value_t *eval_constexpr(token_t *token);

#endif