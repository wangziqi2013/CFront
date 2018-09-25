
#include <ctype.h>
#include "eval.h"
#include "type.h"

// Writes the number of given base into the data area of the token
void eval_getintimm(value_t *val, token_t *token) {
  token_type_t type = token->type;
  char *s = token->s;
  int base;
  switch(token->type) {
    case T_HEX_INT_CONST: s += 2; base = 16; break;
    case T_OCT_INT_CONST: s++; base = 8; break;
    case T_DEC_INT_CONST: base = 10; break;
    default: assert(0);
  }
}

value_t *eval_constexpr(token_t *token) {
  return NULL;
}