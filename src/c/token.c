
#include "token.h"

// Fill an operator token object according to its type
// Return value is the new location after getting the token
char *token_get_op(char *s, token_t *token) {
  assert(*s != '\0');
  switch(s[0]) {
    // Must be single character operator
    case ',': token->type = T_COMMA; return s + 1;                      // ,
    case '(': token->type = T_LCPAREN; return s + 1;                    // (
    case ')': token->type = T_RCPAREN; return s + 1;                    // )
    case '[': token->type = T_LSPAREN; return s + 1;                    // [
    case ']': token->type = T_RSPAREN; return s + 1;                    // ]
    case '{': token->type = T_LCPAREN; return s + 1;                    // {
    case '}': token->type = T_RCPAREN; return s + 1;                    // }
    case '.': token->type = T_DOT; return s + 1;                        // .
    case '?': token->type = T_QMARK; return s + 1;                      // ?
    case ':': token->type = T_COLON; return s + 1;                      // :
    case '~': token->type = T_BIT_COMPLETEMT; return s + 1;             // ~
    // Multi character
    case '-':
      switch(s[1]) {
        case '-': token->type = T_DEC; return s + 2;                    // --
        case '=': token->type = T_MINUS_ASSIGN; return s + 2;           // -=
        case '>': token->type = T_ARROW; return s + 2;                  // ->
        case '\0': 
        default: token->type = T_MINUS; return s + 1;                   // -
      }
    case '+':
      switch(s[1]) {
        case '+': token->type = T_INC; return s + 2;                    // ++
        case '=': token->type = T_PLUS_ASSIGN; return s + 2;            // +=
        case '\0': 
        default: token->type = T_PLUS; return s + 1;                    // +
      }
    case '*':
  }
}