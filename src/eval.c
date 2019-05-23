
#include <ctype.h>
#include "eval.h"
#include "type.h"

// Return value is a mask that needs to be OR'ed onto the decl property of the destination operand
decl_prop_t eval_int_convert(decl_prop_t int1, decl_prop_t int2) {
  assert(BASETYPE_GET(int1) >= BASETYPE_CHAR && BASETYPE_GET(int1) <= BASETYPE_ULLONG);
  assert(BASETYPE_GET(int2) >= BASETYPE_CHAR && BASETYPE_GET(int2) <= BASETYPE_ULLONG);
  int_prop_t p1 = ints[BASETYPE_INDEX(int1)], p2 = ints[BASETYPE_INDEX(int2)];
  // MIN on sign means that we prefer unsigned when the sizes are equal
  int_prop_t ret = {EVAL_MIN(p1.sign, p2.sign), EVAL_MAX(p1.size, p2.size)}; 
  // The sign of the longer type override the sign of shorter type
  if(p1.size > p2.size) ret.sign = p1.sign;
  else if(p2.size > p1.size) ret.sign = p2.sign;
  for(int i = 1;i < (int)sizeof(ints) / (int)sizeof(int_prop_t);i++) 
    if(memcmp(&ints[i], &ret, sizeof(int_prop_t)) == 0) return BASETYPE_FROMINDEX(i);
  assert(0); // Cannot reach here
  return 0;
}

// Take a max bite until the next char one is not legal digit
// Return next char, result in ret variable
char *eval_const_atoi_maxbite(char *s, int base, token_t *token, int *ret) {
  *ret = 0;
  do { 
    char ch = *s;
    int digit;
    if(ch >= '0' && ch <= '9') digit = ch - '0';
    else if(ch >= 'A' && ch <= 'F') digit = ch - 'A' + 10;
    else if(ch >= 'a' && ch <= 'f') digit = ch - 'a' + 10;
    else break; // Invalid character for any base
    if(digit >= base) break; // Invalid character for the base
    *ret = *ret * base + digit;
  } while(*++s);
  return s;
} 

// max_char is the maximum number of characters allowed in the literal; 0 means don't care
int eval_const_atoi(char *s, int base, token_t *token, int max_char) {
  int ret = 0;
  char *end;
  int chars = (end = eval_const_atoi_maxbite(s, base, token, &ret)) - s;
  if(chars == 0) error_row_col_exit(token->offset, "Empty integer literal sequence\n");
  if(max_char && chars > max_char) 
    error_row_col_exit(token->offset, "Maximum of %d digits are allowed in integer constant \"%s\"\n", max_char, token->str);
  if(*end != '\0') error_row_col_exit(token->offset, "Invalid character \'%c\' in integer constant \"%s\"\n", *end, token->str);
  return ret;
} 

char eval_const_char_token(token_t *token) {
  assert(token->type == T_CHAR_CONST && BASETYPE_GET(token->decl_prop) == BASETYPE_CHAR);
  int len = strlen(token->str); // Remaining characters
  if(len == 0) error_row_col_exit(token->offset, "Empty char literal\n");
  if(token->str[0] != '\\') { // Not an escaped character, just return
    if(len != 1) error_row_col_exit(token->offset, "Char literal \'%s\' contains more than one character\n", token->str);
    return token->str[0];
  }
  if(len == 1) error_row_col_exit(token->offset, "Empty escape sequence\n");
  char escaped = token->str[1];
  if(escaped >= '0' && escaped <= '7') return eval_const_atoi(&token->str[1], 8, token, 3); // 3 digits oct
  else if(escaped == 'x') return eval_const_atoi(&token->str[2], 16, token, 2); // 2 digits hex
  if(len != 2) error_row_col_exit(token->offset, "Multi-character unknown escape sequence: \"%s\"\n", token->str);
  switch(escaped) {
    case 'n': return '\n'; break;
    case '0': return '\0'; break;
    case 'r': return '\r'; break;
    case '\\': return '\\'; break;
    case '\'': return '\''; break;
    case '\"': return '\"'; break;
    case 'a': return '\a'; break;
    case 'b': return '\b'; break;
    case 'f': return '\f'; break;
    case 't': return '\t'; break;
    case 'v': return '\v'; break;
    default: error_row_col_exit(token->offset, "Unknown escaped character: %c\n", escaped); break;
  }
  return 0;
}

int eval_const_int_token(token_t *token) {
  char *s = token->str;
  int base;
  switch(token->type) {
    case T_HEX_INT_CONST: base = 16; break;
    case T_OCT_INT_CONST: base = 8; break;
    case T_CHAR_CONST: // Fall through
    case T_DEC_INT_CONST: base = 10; break;
    default: error_row_col_exit(token->offset, "Must be integer constant type in this context\n"); break;
  }
  // Throw warning if literal type is not int (i.e. no U/L modifiers after the literal)
  if(BASETYPE_GET(token->decl_prop) != BASETYPE_INT) 
    warn_row_col_exit(token->offset, 
      "Integer constant will be implicitly converted to \"int\" type in this context (was \"%s\")\n", 
      token_decl_print(token->decl_prop));
  return token->type == T_CHAR_CONST ? (int)eval_const_char_token(token) : eval_const_atoi(s, base, token, 0);
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
    case T_CHAR_CONST:
    case T_DEC_INT_CONST: ret = eval_const_int_token(token); break;
    // sizeof operator (queries the type system)
    case EXP_SIZEOF: // Temporarily disable this
    default: error_row_col_exit(token->offset, 
      "Unsupported token for constant integer expression: \"%s\"\n", token_typestr(token->type));
      break;
  }
  return ret;
}
