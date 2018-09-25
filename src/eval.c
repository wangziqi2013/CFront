
#include <ctype.h>
#include "eval.h"
#include "type.h"

// Writes the number of given base into the data area of the token
void eval_getintimm(value_t *val, token_t *token) {
  char *s = token->str;
  int base;
  switch(token->type) {
    case T_HEX_INT_CONST: base = 16; break;
    case T_OCT_INT_CONST: base = 8; break;
    case T_DEC_INT_CONST: base = 10; break;
    default: assert(0);
  }
  int sz = type_getintsize(token->decl_prop);  // Number of bytes in the type
  if(sz > (int)sizeof(type_maxint_t)) 
    error_row_col_exit(token->offset, "Sorry, do not support integer literal \"%s\" larger than %lu bytes\n", 
                       token->str, sizeof(type_maxint_t));
  type_maxint_t scratch = (type_maxint_t)0;
  do { scratch = scratch * base + \
                 (*s >= 'A' && *s <= 'F') ? (*s - 'A' + 10) : ((*s >= 'a' && *s <= 'f') ? (*s - 'a' + 10) : *s - '0');
  } while(*++s);
  switch(sz) {
    case 1: val->ucharval = (uint8_t)scratch; break;
    case 2: val->ushortval = (uint16_t)scratch; break;
    case 4: val->uintval = (uint32_t)scratch; break;
    case 8: val->ulongval = (uint64_t)scratch; break;
    default: assert(0); // Should already be detected
  }
  return;
}

value_t *eval_constexpr(token_t *token) {
  return NULL;
}