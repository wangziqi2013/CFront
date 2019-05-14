
#include <ctype.h>
#include "eval.h"
#include "type.h"

int eval_const_getintimm(token_t *token) {
  char *s = token->str;
  int base;
  switch(token->type) {
    case T_HEX_INT_CONST: base = 16; break;
    case T_OCT_INT_CONST: base = 8; break;
    case T_DEC_INT_CONST: base = 10; break;
    default: assert(0);
  }
  int ret = 0;
  do { ret = ret * base + \
             (*s >= 'A' && *s <= 'F') ? (*s - 'A' + 10) : ((*s >= 'a' && *s <= 'f') ? (*s - 'a' + 10) : *s - '0');
  } while(*++s);
  return ret;
}

int eval_const(token_t *token) {
  int ret = 0;
  switch(token->type) {
    // Unary operators
    case EXP_PLUS: ret = eval_const(ast_getchild(token, 0)); break;
    case EXP_LOGICAL_NOT: ret = !eval_const(ast_getchild(token, 0)); break;
    case EXP_BIT_NOT: ret = ~eval_const(ast_getchild(token, 0)); break;
    // Binary operators
    case EXP_MUL: ret = eval_const(ast_getchild(token, 0)) * eval_const(ast_getchild(token, 1)); break;
    case EXP_DIV: { 
      int rhs = eval_const(ast_getchild(token, 1));
      if(rhs == 0) error_row_col_exit(token->offset, "The dividend of constant expression is zero\n");
      ret = eval_const(ast_getchild(token, 0)) * eval_const(ast_getchild(token, 1));
    }
    case EXP_MOD: ret = eval_const(ast_getchild(token, 0)) % eval_const(ast_getchild(token, 1));
    // sizeof operator (queries the type system)
    case EXP_SIZEOF: // Temporarily disable this
    default: error_row_col_exit(token->offset, 
      "Unsupported token for constant integer expression: \"%s\"", token_typestr(token->type));
      break;
  }
  return ret;
}

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
  //val->size = sz; // TODO: ADD SIZE
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