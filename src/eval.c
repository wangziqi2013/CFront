
#include <ctype.h>
#include "eval.h"
#include "type.h"

int eval_const_int_getintimm(token_t *token) {
  char *s = token->str;
  int base;
  // We only support evaluating integer type (i.e. no U/L modifiers after the literal)
  if(BASETYPE_GET(token->decl_prop) != BASETYPE_INT) 
    error_row_col_exit(token->offset, "Integer constant must be of \"int\" type in this context\n");
  switch(token->type) {
    case T_HEX_INT_CONST: base = 16; break;
    case T_OCT_INT_CONST: base = 8; break;
    case T_DEC_INT_CONST: base = 10; break;
    default: assert(0);
  }
  int ret = 0;
  do { ret = ret * base + \
             ((*s >= 'A' && *s <= 'F') ? (*s - 'A' + 10) : ((*s >= 'a' && *s <= 'f') ? (*s - 'a' + 10) : *s - '0'));
  } while(*++s);
  return ret;
}

int eval_const_int(token_t *token) {
  int ret = 0;
  switch(token->type) {
    // Unary operators
    case EXP_PLUS: ret = eval_const_int(ast_getchild(token, 0)); break;
    case EXP_LOGICAL_NOT: ret = !eval_const_int(ast_getchild(token, 0)); break;
    case EXP_BIT_NOT: ret = ~eval_const_int(ast_getchild(token, 0)); break;
    // Binary operators
    case EXP_MUL: ret = eval_const_int(ast_getchild(token, 0)) * eval_const_int(ast_getchild(token, 1)); break;
    case EXP_DIV: { 
      int rhs = eval_const_int(ast_getchild(token, 1));
      if(rhs == 0) error_row_col_exit(token->offset, "The dividend of constant expression \"%s\" is zero\n", token_typestr(token->type));
      ret = eval_const_int(ast_getchild(token, 0)) / rhs;
      break;
    }
    case EXP_MOD: { 
      int rhs = eval_const_int(ast_getchild(token, 1));
      if(rhs == 0) error_row_col_exit(token->offset, "The dividend of constant expression \"%s\" is zero\n", token_typestr(token->type));
      ret = eval_const_int(ast_getchild(token, 0)) % rhs;
      break;
    }
    case EXP_ADD: ret = eval_const_int(ast_getchild(token, 0)) + eval_const_int(ast_getchild(token, 1)); break;
    case EXP_SUB: ret = eval_const_int(ast_getchild(token, 0)) - eval_const_int(ast_getchild(token, 1)); break;
    case EXP_LSHIFT: ret = eval_const_int(ast_getchild(token, 0)) << eval_const_int(ast_getchild(token, 1)); break;
    case EXP_RSHIFT: ret = eval_const_int(ast_getchild(token, 0)) >> eval_const_int(ast_getchild(token, 1)); break;
    case EXP_LESS: ret = eval_const_int(ast_getchild(token, 0)) < eval_const_int(ast_getchild(token, 1)); break;
    case EXP_GREATER: ret = eval_const_int(ast_getchild(token, 0)) > eval_const_int(ast_getchild(token, 1)); break;
    case EXP_LEQ: ret = eval_const_int(ast_getchild(token, 0)) <= eval_const_int(ast_getchild(token, 1)); break;
    case EXP_GEQ: ret = eval_const_int(ast_getchild(token, 0)) >= eval_const_int(ast_getchild(token, 1)); break;
    case EXP_EQ: ret = eval_const_int(ast_getchild(token, 0)) == eval_const_int(ast_getchild(token, 1)); break;
    case EXP_NEQ: ret = eval_const_int(ast_getchild(token, 0)) != eval_const_int(ast_getchild(token, 1)); break;
    case EXP_BIT_AND: ret = eval_const_int(ast_getchild(token, 0)) & eval_const_int(ast_getchild(token, 1)); break;
    case EXP_BIT_OR: ret = eval_const_int(ast_getchild(token, 0)) | eval_const_int(ast_getchild(token, 1)); break;
    case EXP_BIT_XOR: ret = eval_const_int(ast_getchild(token, 0)) ^ eval_const_int(ast_getchild(token, 1)); break;
    case EXP_LOGICAL_AND: ret = eval_const_int(ast_getchild(token, 0)) && eval_const_int(ast_getchild(token, 1)); break;
    case EXP_LOGICAL_OR: ret = eval_const_int(ast_getchild(token, 0)) || eval_const_int(ast_getchild(token, 1)); break;
    case EXP_COMMA: ret = eval_const_int(ast_getchild(token, 0)), eval_const_int(ast_getchild(token, 1)); break;
    // Tenary operator
    case EXP_COND: ret = \
      eval_const_int(ast_getchild(token, 0)) ? eval_const_int(ast_getchild(token, 1)) : eval_const_int(ast_getchild(token, 2)); break;
    // Immediate values (integer, char expanded into integers)
    case T_HEX_INT_CONST:
    case T_OCT_INT_CONST:
    case T_DEC_INT_CONST: ret = eval_const_int_getintimm(token); break;
    // sizeof operator (queries the type system)
    case EXP_SIZEOF: // Temporarily disable this
    default: error_row_col_exit(token->offset, 
      "Unsupported token for constant integer expression: \"%s\"\n", token_typestr(token->type));
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
                 ((*s >= 'A' && *s <= 'F') ? (*s - 'A' + 10) : ((*s >= 'a' && *s <= 'f') ? (*s - 'a' + 10) : *s - '0'));
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

value_t *eval_const_intexpr(token_t *token) {
  return NULL;
}