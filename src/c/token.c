
#include "token.h"

// Fill an operator token object according to its type
// Return value is the new location after getting the token
char *token_get_op(const char *s, token_t *token) {
  assert(*s != '\0');
  switch(s[0]) {
    case ',': token->type = T_COMMA; return s + 1;
    case '(': token->type = T_LCPAREN; return s + 1;
    case ')': token->type = T_RCPAREN; return s + 1;
    case '[': token->type = T_LSPAREN; return s + 1;
    case ']': token->type = T_RSPAREN; return s + 1;
    case '{': token->type = T_LCPAREN; return s + 1;
    case '}': token->type = T_RCPAREN; return s + 1;
    case '.': token->type = T_DOT; return s + 1;
    case '?': token->type = T_QMARK; return s + 1;
    case ':': token->type = T_COLON; return s + 1;
    case '~': token->type = T_BIT_COMPLETEMT; return s + 1;
    default: assert(0);
  }
}