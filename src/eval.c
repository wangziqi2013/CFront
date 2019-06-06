
#include <ctype.h>
#include "eval.h"
#include "type.h"

// The following functions perform constant evaluation
// value type is not altered

/*
// If signed == 1 and to > from, it is sign extension
void eval_const_adjust_size(value_t *value, int to, int from, int signed) {
  assert(to <= TYPE_INT_SIZE_MAX && to > 0 && from <= TYPE_INT_SIZE_MAX && from > 0);
  //value_t *ret = value_init();
  if(from == to) return;
  if(to < from) { // truncation

  } else { // Extension

  }
  return;
}
*/

// Represent a character as \xhh
char *eval_hex_char(char ch) {
  static char buffer[5];
  if(isprint(ch)) sprintf(buffer, "%c", ch);
  else sprintf(buffer, "\\x%02X", (unsigned char)ch); // Use a cast to avoid sign extension
  return buffer;
}

// Caller frees the returned string object
str_t *eval_print_const_str(str_t *s) {
  char *ptr = str_cstr(s);
  str_t *ret = str_init();
  while(*ptr) {
    if(isprint(*ptr)) {
      str_append(ret, *ptr);
    } else {
      switch(*ptr) {
        case '\n': str_concat(ret, "\\n"); break;
        case '\r': str_concat(ret, "\\r"); break;
        case '\a': str_concat(ret, "\\a"); break;
        case '\b': str_concat(ret, "\\b"); break;
        case '\f': str_concat(ret, "\\f"); break;
        case '\t': str_concat(ret, "\\t"); break;
        case '\v': str_concat(ret, "\\v"); break;
        case '\'': str_concat(ret, "\\'"); break;
        case '\"': str_concat(ret, "\\\""); break;
        case '\\': str_concat(ret, "\\\\"); break;
        default: {
          str_concat(ret, eval_hex_char(*ptr));
          break;
        } // default
      } // switch
    } // is printable or not
    ptr++;
  } // while(*ptr)
  return ret;
}

// Take a max bite until the next char one is not legal digit or string ends
// Return next char ptr (could be pointing to '\0'), result in ret variable
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

// Argument max_char is the maximum number of characters allowed in the literal; 0 means don't care
// Argument check_end indicates whether we enforce the next reading position to be '\0'; 0 means don't care
// Argument next returns the next char to read in s; NULL will be ignored
int eval_const_atoi(char *s, int base, token_t *token, int max_char, int check_end, char **next) {
  int ret = 0;
  char *end;
  int chars = (end = eval_const_atoi_maxbite(s, base, token, &ret)) - s;
  if(chars == 0) error_row_col_exit(token->offset, "Empty integer literal sequence\n");
  if(max_char && chars > max_char) 
    error_row_col_exit(token->offset, "Maximum of %d digits are allowed in integer constant \"%s\"\n", max_char, token->str);
  if(check_end && *end != '\0') 
    error_row_col_exit(token->offset, "Invalid character \'%s\' in integer constant \"%s\"\n", eval_hex_char(*end), token->str);
  if(next) *next = end; // Return next char to read in this argument if it is not NULL
  return ret;
} 

// Converts escaped sequence into a char, i.e. input 'n' will return '\n'
// Do not process 'x', and 0 - 7
// Argument "token" is just for error reporting
char eval_escaped_char(char escaped, token_t *token) {
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
    default: error_row_col_exit(token->offset, "Unknown escaped character: \'%s\'\n", eval_hex_char(escaped)); break;
  }
  assert(0);
  return 0; // Should never reach here
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
  if(escaped >= '0' && escaped <= '7') {
    return eval_const_atoi(&token->str[1], 8, token, 3, ATOI_CHECK_END, NULL);  // 3 digits oct
  } else if(escaped == 'x') {
    return eval_const_atoi(&token->str[2], 16, token, 2, ATOI_CHECK_END, NULL); // 2 digits hex
  }
  if(len != 2) error_row_col_exit(token->offset, "Multi-character unknown escape sequence: \"%s\"\n", token->str);
  return eval_escaped_char(escaped, token); // Regular single char escape
}

// Given a string liternal token, return a string object containing binary data of the string
str_t *eval_const_str_token(token_t *token) {
  assert(token->type == T_STR_CONST && token->str);
  str_t *s = str_init();
  char *ptr = token->str;
  while(*ptr) { // it is possible that the string is empty
    char ch0 = *ptr;
    if(ch0 != '\\') { 
      str_append(s, ch0); 
      ptr++;
    } else {
      char ch1 = ptr[1];   // ch1 is the escaped character
      assert(ch1 != '\0'); // Otherwise the string ends with '\' which will escape the following '"'
      char escaped_value;
      if(ch1 == 'x') {
        escaped_value = eval_const_atoi(ptr + 2, 16, token, 2, ATOI_NO_CHECK_END, &ptr); // Must have 1 or 2 char
      } else if(ch1 >= '0' && ch1 <= '7') {
        escaped_value = eval_const_atoi(ptr + 1, 8, token, 3, ATOI_NO_CHECK_END, &ptr);  // Must have 1 - 3 char
      } else {
        escaped_value = eval_escaped_char(ptr[1], token);
        ptr += 2;
      }
      str_append(s, escaped_value);
    }
  }
  return s;
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
  return token->type == T_CHAR_CONST ? \
    (int)eval_const_char_token(token) : eval_const_atoi(s, base, token, ATOI_NO_MAX_CHAR, ATOI_CHECK_END, NULL);
}

int eval_const_int(type_cxt_t *cxt, token_t *token) {
  int ret = 0;
  switch(token->type) {
    // Unary operators
    case EXP_PLUS: ret = eval_const_int(cxt, ast_getchild(token, 0)); break;
    case EXP_LOGICAL_NOT: ret = !eval_const_int(cxt, ast_getchild(token, 0)); break;
    case EXP_BIT_NOT: ret = ~eval_const_int(cxt, ast_getchild(token, 0)); break;
    // Binary operators
    case EXP_MUL: ret = eval_const_int(cxt, ast_getchild(token, 0)) * eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_DIV: { 
      int rhs = eval_const_int(cxt, ast_getchild(token, 1));
      if(rhs == 0) error_row_col_exit(token->offset, "The dividend of constant expression \"%s\" is zero\n", token_typestr(token->type));
      ret = eval_const_int(cxt, ast_getchild(token, 0)) / rhs;
      break;
    }
    case EXP_MOD: { 
      int rhs = eval_const_int(cxt, ast_getchild(token, 1));
      if(rhs == 0) error_row_col_exit(token->offset, "The dividend of constant expression \"%s\" is zero\n", token_typestr(token->type));
      ret = eval_const_int(cxt, ast_getchild(token, 0)) % rhs;
      break;
    }
    case EXP_ADD: ret = eval_const_int(cxt, ast_getchild(token, 0)) + eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_SUB: ret = eval_const_int(cxt, ast_getchild(token, 0)) - eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_LSHIFT: ret = eval_const_int(cxt, ast_getchild(token, 0)) << eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_RSHIFT: ret = eval_const_int(cxt, ast_getchild(token, 0)) >> eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_LESS: ret = eval_const_int(cxt, ast_getchild(token, 0)) < eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_GREATER: ret = eval_const_int(cxt, ast_getchild(token, 0)) > eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_LEQ: ret = eval_const_int(cxt, ast_getchild(token, 0)) <= eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_GEQ: ret = eval_const_int(cxt, ast_getchild(token, 0)) >= eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_EQ: ret = eval_const_int(cxt, ast_getchild(token, 0)) == eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_NEQ: ret = eval_const_int(cxt, ast_getchild(token, 0)) != eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_BIT_AND: ret = eval_const_int(cxt, ast_getchild(token, 0)) & eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_BIT_OR: ret = eval_const_int(cxt, ast_getchild(token, 0)) | eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_BIT_XOR: ret = eval_const_int(cxt, ast_getchild(token, 0)) ^ eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_LOGICAL_AND: ret = eval_const_int(cxt, ast_getchild(token, 0)) && eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_LOGICAL_OR: ret = eval_const_int(cxt, ast_getchild(token, 0)) || eval_const_int(cxt, ast_getchild(token, 1)); break;
    case EXP_COMMA: ret = eval_const_int(cxt, ast_getchild(token, 0)), eval_const_int(cxt, ast_getchild(token, 1)); break;
    // Tenary operator
    case EXP_COND: ret = \
      eval_const_int(cxt, ast_getchild(token, 0)) ? eval_const_int(cxt, ast_getchild(token, 1)) : eval_const_int(cxt, ast_getchild(token, 2)); break;
    // Immediate values (integer, char expanded into integers)
    case T_HEX_INT_CONST:
    case T_OCT_INT_CONST:
    case T_CHAR_CONST:
    case T_DEC_INT_CONST: ret = eval_const_int_token(token); break;
    case T_IDENT: {
      value_t *value = (value_t *)scope_search(cxt, SCOPE_VALUE, token->str);
      if(!value) error_row_col_exit(token->offset, "Cannot find integer constant \"%s\"\n", token->str);
      if(value->addrtype != ADDR_IMM) error_row_col_exit(token->offset, "Name \"%s\" is not a constant\n", token->str);
      if(!type_is_int(value->type)) error_row_col_exit(token->offset, "Name \"%s\" is not of integer type\n", token->str);
      if(BASETYPE_GET(value->type->decl_prop) != BASETYPE_INT) 
        warn_row_col_exit(token->offset, "Name \"%s\" will be converted to int type\n", token->str);
      ret = value->int32;
      break;
    }
    case EXP_CAST: {
      token_t *decl = ast_getchild(token, 1);
      token_t *basetype = ast_getchild(decl, 0);
      type_t *cast_type = type_gettype(cxt, decl, basetype, 0); // Do not allow void and storage class
      if(!type_is_int(cast_type)) error_row_col_exit(token->offset, "Can only cast to integer type\n");
      if(BASETYPE_GET(cast_type->decl_prop) != BASETYPE_INT) 
        warn_row_col_exit(token->offset, "Type cast will be ignored; Result will be integer type\n");
      ret = eval_const_int(cxt, ast_getchild(token, 0));
      break;
    }
    // sizeof operator (queries the type system and/or symbol table)
    case EXP_SIZEOF: {
      token_t *decl = ast_getchild(token, 0);
      assert(decl);
      if(decl->type == T_DECL) {
        token_t *basetype = ast_getchild(decl, 0);
        assert(basetype);
        type_t *type = type_gettype(cxt, decl, basetype, TYPE_ALLOW_VOID); // Allow void but not storage class
        if(type->size == TYPE_UNKNOWN_SIZE) error_row_col_exit(token->offset, "Sizeof operator with an incomplete type\n");
        ret = type->size;
      } else {
        token_t *exp = decl; // If not then must be an expression
        type_t *type = type_typeof(cxt, exp, TYPEOF_IGNORE_FUNC_ARG | TYPEOF_IGNORE_ARRAY_INDEX);
        ret = type->size;
      }
      break;
    }
    default: error_row_col_exit(token->offset, 
      "Unsupported token for constant integer expression: \"%s\"\n", token_typestr(token->type));
      break;
  }
  return ret;
}
