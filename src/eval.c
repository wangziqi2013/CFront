
#include <ctype.h>
#include "eval.h"
#include "type.h"

// Indicex are number of bytes in the number; 0UL is undefined
uint64_t eval_int_masks[9] = {
  0UL, 0xFFUL, 0xFFFFUL, 0UL, 0xFFFFFFFFUL, 0UL, 0UL, 0UL, 0xFFFFFFFFFFFFFFFF, // 0 - 8
};

// The following functions perform constant evaluation
// value type is not altered

uint64_t eval_const_get_mask(int size) {
  assert(size <= TYPE_INT_SIZE_MAX && size > 0);
  assert(size <= EVAL_MAX_CONST_SIZE);
  uint64_t op_mask = eval_int_masks[size];
  assert(op_mask);
  return op_mask;
}

uint64_t eval_const_get_sign_mask(int size) {
  uint64_t sign_mask = eval_int_masks[size];
  sign_mask -= (sign_mask >> 1);
  return sign_mask;
}

// Returns 1 if the underlying literal is zero
int eval_const_is_zero(value_t *value, int size) {
  uint64_t mask = eval_const_get_mask(size);
  return (value->uint64 & mask) == 0;
}

// If signed == 1 and to > from, it is sign extension; We do not use or change value->type
// Returns the value itself
uint64_t eval_const_adjust_size(value_t *value, int to, int from, int is_signed) {
  uint64_t to_mask = eval_const_get_mask(to), from_mask = eval_const_get_mask(from);
  uint64_t ret = value->uint64;
  if(from == to) {
    return ret;
  } else if(to < from) { // Truncation
    ret &= to_mask;
  } else { // Extension
    uint64_t from_sign_mask = eval_const_get_sign_mask(from);
    if((ret & from_sign_mask) && is_signed) { // Sign extension
      ret |= (to_mask - from_mask);
    } else { // Zero extension
      ret &= from_mask; // Clear higher bits
    }
  }
  return ret;
}

// Return NULL if operation overflows; Return result raw binary representation
uint64_t eval_const_add(value_t *op1, value_t *op2, int size, int is_signed, int *overflow) {
  *overflow = 0;
  uint64_t mask = eval_const_get_mask(size);
  uint64_t result = ((op1->uint64 & mask) + (op2->uint64 & mask)) & mask;
  // If result is less than one of them, we have seen a carry
  int carry = ((result & mask) < (op1->uint64 & mask)) || ((result & mask) < (op2->uint64 & mask)); 
  if(!is_signed && carry) { // Overflow - unsigned addition, and we see a carray bit
    *overflow = 1;
  } else if(is_signed) {
    uint64_t sign_mask = eval_const_get_sign_mask(size);
    uint64_t op1_sign = op1->uint64 & sign_mask;
    uint64_t op2_sign = op2->uint64 & sign_mask;
    uint64_t result_sign = result & sign_mask;
    if(op1_sign && op2_sign && !result_sign) *overflow = 1; // Underflow - neg + neg = pos
    else if(!op1_sign && !op2_sign && result_sign) *overflow = 1; // Overflow - pos + pos = neg
  }
  return result;
}

uint64_t eval_const_sub(value_t *op1, value_t *op2, int size, int is_signed, int *overflow) {
  *overflow = 0;
  uint64_t mask = eval_const_get_mask(size);
  if(!is_signed) {
    if((op1->uint64 & mask) < (op2->uint64 & mask)) *overflow = 1; // Unsigned overflow
    return ((op1->uint64 & mask) - (op2->uint64 & mask)) & mask;
  }
  value_t temp;
  temp.uint64 = (~op2->uint64 + 1) & mask; // 2's complement
  return eval_const_add(op1, &temp, size, is_signed, overflow);
}

uint64_t eval_const_mul(value_t *op1, value_t *op2, int size, int is_signed, int *overflow) {
  *overflow = 0;
  int final_invert_sign = 0;
  __uint128_t mask = (__uint128_t)eval_const_get_mask(size);
  __uint128_t sign_mask = (__uint128_t)eval_const_get_sign_mask(size);
  __uint128_t op1_value = (__uint128_t)op1->uint64 & mask;
  __uint128_t op2_value = (__uint128_t)op2->uint64 & mask;
  if(op1_value == 0 || op2_value == 0) return 0UL; // Special handling for zero
  uint64_t op1_sign = op1_value & sign_mask;
  uint64_t op2_sign = op2_value & sign_mask;
  if(is_signed && op1_sign) { op1_value = (~op1_value + 1) & mask; final_invert_sign++; }
  if(is_signed && op2_sign) { op2_value = (~op2_value + 1) & mask; final_invert_sign++; }
  final_invert_sign %= 2; // This is 1 if we need to invert the sign after MUL
  __uint128_t result = op1_value * op2_value;
  if(result & ~mask) *overflow = 1;
  else if(is_signed && (result & sign_mask)) *overflow = 1; // Result is negative number
  result &= mask;
  return final_invert_sign ? (~result + 1) & mask : result;
}

// First argument controls whether it is div or mod
uint64_t eval_const_div_mod(int is_div, value_t *op1, value_t *op2, int size, int is_signed, int *div_zero) {
  *div_zero = 0;
  int final_invert_sign = 0;
  __uint128_t mask = (__uint128_t)eval_const_get_mask(size);
  __uint128_t sign_mask = (__uint128_t)eval_const_get_sign_mask(size);
  __uint128_t op1_value = (__uint128_t)op1->uint64 & mask;
  __uint128_t op2_value = (__uint128_t)op2->uint64 & mask;
  if(op1_value == 0 || op2_value == 0) return 0UL; // Special handling for zero
  uint64_t op1_sign = op1_value & sign_mask;
  uint64_t op2_sign = op2_value & sign_mask;
  if(is_signed && op1_sign) { op1_value = (~op1_value + 1) & mask; final_invert_sign++; }
  if(is_signed && op2_sign) { op2_value = (~op2_value + 1) & mask; final_invert_sign++; }
  final_invert_sign %= 2; // This is 1 if we need to invert the sign after MUL
  if(op2_value == (__uint128_t)0UL) { // Check div by zero error and returns 0 if it is the case
    *div_zero = 1; 
    return 0;
  }
  __uint128_t result;
  if(is_div) result = (op1_value / op2_value) & mask;
  else result = (op1_value % op2_value) & mask;
  return final_invert_sign ? (uint64_t)((~result + 1) & mask) : (uint64_t)result;
}

// If first arg is 1 then we left shift and ignore sign; Otherwise right shift and may propagate sign bit
// op2 is always treated as an unsigned number; If it is larger than size of op1, and it is left shift or right unsigned shift
// we set overflow flag to 1
uint64_t eval_const_shift(int is_left, value_t *op1, value_t *op2, int size, int is_signed, int *shift_overflow) {
  *shift_overflow = 0;
  uint64_t mask = eval_const_get_mask(size);
  uint64_t sign_mask = eval_const_get_sign_mask(size);
  uint64_t op1_value = op1->uint64 & mask;
  uint64_t op2_value = op2->uint64 & mask;
  uint64_t op1_sign = op1_value & sign_mask;
  if(op2_value >= 8 * (uint64_t)size) { // Note that we compare bit size
    if(is_left || (!is_left && !is_signed)) { // Always result in zero
      *shift_overflow = 1;
      return 0UL;
    } else if(!is_left && is_signed) {
      return op1_sign ? mask : 0UL; // All 1's, because the sign bit
    }
  }
  if(is_left) op1_value <<= op2_value;
  else op1_value >>= op2_value; // This is unsigned shift
  if(!is_left && op1_sign && is_signed) op1_value |= (mask - (mask >> op2_value)); // Fill high bits with all 1's
  return op1_value & mask;
}

// Note: This function returns integer because logical operations always returns integer
// The first argument indicates the type of operation
int eval_const_cmp(token_type_t op, value_t *op1, value_t *op2, int size, int is_signed) {
  uint64_t mask = eval_const_get_mask(size);
  uint64_t op1_value = op1->uint64 & mask;
  uint64_t op2_value = op2->uint64 & mask;
  int ret = 0;
  int shift_bits = (8 * (TYPE_LONG_SIZE - size)); // We shift all valid bits of this size to MSB and use native cmp
  op1_value <<= shift_bits;
  op2_value <<= shift_bits;
  switch(op) {
    case EXP_LESS: ret = is_signed ? op1_value < op2_value : (int64_t)op1_value < (int64_t)op2_value; break;
    case EXP_LEQ: ret = is_signed ? op1_value <= op2_value : (int64_t)op1_value <= (int64_t)op2_value; break;
    case EXP_GREATER: ret = is_signed ? op1_value > op2_value : (int64_t)op1_value > (int64_t)op2_value; break;
    case EXP_GEQ: ret = is_signed ? op1_value >= op2_value : (int64_t)op1_value >= (int64_t)op2_value; break;
    case EXP_EQ: ret = op1_value == op2_value; break; // == and != ignores sign
    case EXP_NEQ: ret = op1_value != op2_value; break;
    default: assert(0); break;
  }
  return ret;
}

// Binary bitwise operations: AND/OR/XOR
uint64_t eval_const_bitwise(token_type_t op, value_t *op1, value_t *op2, int size) {
  uint64_t mask = eval_const_get_mask(size);
  uint64_t op1_value = op1->uint64 & mask;
  uint64_t op2_value = op2->uint64 & mask;
  uint64_t ret = 0;
  switch(op) {
    case EXP_BIT_AND: ret = op1_value & op2_value; break;
    case EXP_BIT_OR: ret = op1_value | op2_value; break;
    case EXP_BIT_XOR: ret = op1_value ^ op2_value; break;
    default: assert(0); break;
  }
  return ret & mask;
}

// Unary operator: logical AND, bitwise AND, negate, plus (which does not change the value)
// Note that logical not returns uint64_t which is different from comparison
uint64_t eval_const_unary(token_type_t op, value_t *value, int size) {
  uint64_t mask = eval_const_get_mask(size);
  uint64_t ret = 0UL;
  switch(op) {
    case EXP_PLUS: ret = value->uint64 & mask; break;
    case EXP_MINUS: ret = (~value->uint64 + 1) & mask; break;
    case EXP_BIT_NOT: ret = ~value->uint64 & mask; break;
    case EXP_LOGICAL_NOT: ret = !value->uint64; break;
    default: assert(0); break;
  }
  return ret;
}

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

value_t *eval_const_get_int_value(type_cxt_t *cxt, token_t *token) {
  assert(BASETYPE_GET(token->decl_prop) >= BASETYPE_CHAR && BASETYPE_GET(token->decl_prop) <= BASETYPE_ULLONG);
  char *s = token->str;
  int base;
  switch(token->type) {
    case T_HEX_INT_CONST: base = 16; break;
    case T_OCT_INT_CONST: base = 8; break;
    case T_CHAR_CONST: // Fall through
    case T_DEC_INT_CONST: base = 10; break;
    default: assert(0); break;
  }
  value_t *value = value_init(cxt);
  value->addrtype = ADDR_IMM;
  value->type = type_init_from(cxt, &type_builtin_ints[BASETYPE_INDEX(token->decl_prop)], token->offset);
  if(token->type == T_CHAR_CONST) { // Char const is directly evaluated because we know the size
    assert(token->decl_prop == BASETYPE_CHAR);
    value->int8 = (int8_t)eval_const_char_token(token);
  } else {
    // Build the integer using value_t arithmetic
    decl_prop_t basetype = BASETYPE_GET(token->decl_prop);
    int_prop_t prop = ints[BASETYPE_INDEX(basetype)];
    int size = prop.size; int sign = prop.sign;
    if(size > EVAL_MAX_CONST_SIZE)
      error_row_col_exit(token->offset, "Currently only support constants within %d bytes\n", EVAL_MAX_CONST_SIZE);
    value_t base_value, digit_value;
    base_value.int32 = base;
    int already_warned = 0;
    while(*s) {
      char ch = *s;
      int digit = 999; // Always > base if none of the below satisfies
      if(ch >= '0' && ch <= '9') digit = (int)(ch - '0');
      else if(ch >= 'A' && ch <= 'F') digit = (int)(ch - 'A' + 10);
      else if(ch >= 'a' && ch <= 'f') digit = (int)(ch - 'a' + 10);
      if(digit >= base) 
        error_row_col_exit(token->offset, "Invalid character '%s' for base %d\n", eval_hex_char(ch), base);
      digit_value.int32 = digit;
      int of1, of2;
      value->uint64 = eval_const_mul(value, &base_value, size, sign, &of1);
      value->uint64 = eval_const_add(value, &digit_value, size, sign, &of2);
      if(!already_warned && (of1 || of2)) {
        warn_row_col_exit(token->offset, "Integer literal \"%s\" overflows for type \"%s\"\n", 
          token->str, token_decl_print(token->decl_prop));
        already_warned = 1;
      }
      s++;
    }
  }
  return value;
}

// This function evaluates a constant expression
value_t *eval_const_exp(type_cxt_t *cxt, token_t *exp) {
  // Leaf types: Integer literal, string literal and identifiers
  if(BASETYPE_GET(exp->decl_prop) >= BASETYPE_CHAR && BASETYPE_GET(exp->decl_prop) <= BASETYPE_ULLONG) {
    return eval_const_get_int_value(cxt, exp);
  } else if(exp->type == T_STR_CONST) { // I tested with GCC and it does not support this, so let's keep it consistent
    error_row_col_exit(exp->offset, "String literal is not allowed in a constant expression\n");
  } else if(BASETYPE_GET(exp->decl_prop)) {  // Unsupported base type literal
    type_error_not_supported(exp->offset, exp->decl_prop);
  } else if(exp->type == T_IDENT) { // Might be of type ADDR_IMM, in which case we take int32
    value_t *value = scope_search(cxt, SCOPE_VALUE, exp->str);
    if(!value) {
      error_row_col_exit(exp->offset, "Name \"%s\" does not exist in current scope\n", exp->str);
    } else if(value->addrtype != ADDR_IMM) {
      error_row_col_exit(exp->offset, "Name \"%s\" is not a compile-time constant\n", exp->str);
    }
    // Make a copy and return - we may modify this object, so a copy is needed
    value_t *ret = value_init(cxt);
    ret->int32 = value->int32;
    ret->type = value->type;
    ret->addrtype = ADDR_IMM;
    return ret;
  }

  // For supported binary operands, first determine result type, and cast operands
  // to that type
  token_t *op1 = ast_getchild(exp, 0);
  token_t *op2 = ast_getchild(exp, 1); // Might be NULL for unary operators
  value_t *op1_value = NULL, *op2_value = NULL; // Operand values of binary operators (after cast)
  type_t *target_type = NULL;          // Type of both operands after convension
  switch(exp->type) {
    case EXP_ADD: case EXP_SUB: case EXP_MUL: case EXP_DIV: case EXP_MOD:
    case EXP_LSHIFT: case EXP_RSHIFT:
    case EXP_LESS: case EXP_LEQ: case EXP_GREATER: case EXP_GEQ:
    case EXP_EQ: case EXP_NEQ: case EXP_BIT_AND: case EXP_BIT_OR: case EXP_BIT_XOR: {
      assert(op1 && op2);
      op1_value = eval_const_exp(cxt, op1);
      op2_value = eval_const_exp(cxt, op2);
      if(!type_is_int(op1_value->type) || !type_is_int(op2_value->type))
        error_row_col_exit(exp->offset, "Consant expression operator must only have integer operands\n");
      target_type = type_int_convert(op1_value->type, op2_value->type);
      int op1_cast = type_cast(target_type, op1_value->type, TYPE_CAST_IMPLICIT, op1->offset);
      int op2_cast = type_cast(target_type, op2_value->type, TYPE_CAST_IMPLICIT, op2->offset);
      int op1_signed = op1_cast == TYPE_CAST_SIGN_EXT;
      int op2_signed = op2_cast == TYPE_CAST_SIGN_EXT;
      op1_value->uint64 = eval_const_adjust_size(op1_value, target_type->size, op1_value->type->size, op1_signed);
      op2_value->uint64 = eval_const_adjust_size(op2_value, target_type->size, op2_value->type->size, op2_signed);
      op1_value->type = target_type;
      op2_value->type = target_type;
    } break;
    case EXP_COND: { // It has three operands: op1 (cond), op2, op3
      token_t *op3 = ast_getchild(exp, 2);
      assert(op1 && op2 && op3);
      op1_value = eval_const_exp(cxt, op1);
      op2_value = eval_const_exp(cxt, op2);
      if(!type_is_int(op1_value->type)) // Only check the condition; op2 and op3 will be checked by upper levels
        error_row_col_exit(exp->offset, "Consant expression operator must only have integer operands\n");
      value_t *op3_value = eval_const_exp(cxt, op3);
      if(type_cmp(op3_value->type, op2_value->type) != TYPE_CMP_EQ) 
        error_row_col_exit(exp->offset, "Condition operator must return two identical types\n");
      int cond = eval_const_is_zero(op1_value, op1_value->type->size);
      if(cond) return op3_value;
      else return op2_value;
    } break;
    case EXP_PLUS: case EXP_MINUS: case EXP_BIT_NOT: case EXP_LOGICAL_NOT: {
      assert(op1);
      op1_value = eval_const_exp(cxt, op1);
      if(!type_is_int(op1_value->type))
        error_row_col_exit(exp->offset, "Consant expression operator must only have integer operands\n");
      op1_value->uint64 = eval_const_unary(exp->type, op1_value, op1_value->type->size);
      return op1_value;
    } break;
    case EXP_CAST: {
      token_t *decl_token = ast_getchild(exp, 1); // This is the second child
      token_t *basetype_token = ast_getchild(decl_token, 0);
      // Allow casting to void or functions returning void
      type_t *target = type_gettype(cxt, decl_token, basetype_token, TYPE_ALLOW_VOID); 
      op1_value = eval_const_to_type(cxt, op1, target, TYPE_CAST_EXPLICIT);
      return op1_value;
    } break;
    case EXP_SIZEOF: {
      assert(op1);
      type_t *type = NULL;
      if(op1->type == T_DECL) {
        token_t *basetype = ast_getchild(op1, 0);
        assert(basetype);
        type = type_gettype(cxt, op1, basetype, TYPE_ALLOW_VOID); // Allow void but not storage class
        if(type_is_void(type)) {
          error_row_col_exit(exp->offset, "\"void\" cannot be used in sizeof() expression\n");
        } else if(type_is_func(type)) {
          error_row_col_exit(exp->offset, "Function type cannot be used in sizeof() expression\n");
        } else if(type->size == TYPE_UNKNOWN_SIZE) {
          error_row_col_exit(exp->offset, "Sizeof operator with an incomplete type\n");
        }
      } else {
        type = type_typeof(cxt, op1, TYPEOF_IGNORE_FUNC_ARG | TYPEOF_IGNORE_ARRAY_INDEX);
      }
      value_t *value = value_init(cxt);
      value->addrtype = ADDR_IMM;
      value->type = type_getint(TYPE_SIZEOF_TYPE);
      value->uint64 = type->size;
      return value;
    } break;
    default: error_row_col_exit(exp->offset, "Operator \"%s\" is not supported for constant expression\n", 
      token_symstr(exp->type));
      break;
  }
  
  value_t *ret = value_init(cxt); // Value will be set in switch statement
  ret->addrtype = ADDR_IMM;
  ret->type = target_type; // This might be changed below in case branches
  int target_size = (int)target_type->size;
  printf("exp %s target type %s target size %d\n", token_typestr(exp->type), type_print_str(0, target_type, 0, 0), target_size);
  printf("op1 0x%lX op2 0x%lX\n", op1_value->uint64, op2_value->uint64);
  int flag = 0; // Overflow or div-by-zero
  int is_signed = type_is_signed(target_type);
  assert(op1_value && op2_value);
  switch(exp->type) {
    case EXP_ADD: {
      ret->uint64 = eval_const_add(op1_value, op2_value, target_size, is_signed, &flag);
      if(flag) warn_row_col_exit(exp->offset, "Operator '+' overflows during constant evaluation\n");
    } break;
    case EXP_SUB: {
      ret->uint64 = eval_const_sub(op1_value, op2_value, target_size, is_signed, &flag);
      if(flag) warn_row_col_exit(exp->offset, "Operator '-' overflows during constant evaluation\n");
    } break;
    case EXP_MUL: {
      ret->uint64 = eval_const_mul(op1_value, op2_value, target_size, is_signed, &flag);
      if(flag) warn_row_col_exit(exp->offset, "Operator '*' overflows during constant evaluation\n");
    } break;
    case EXP_DIV: case EXP_MOD: {
      ret->uint64 = eval_const_div_mod(exp->type == EXP_DIV, op1_value, op2_value, target_size, is_signed, &flag);
      if(flag) warn_row_col_exit(exp->offset, "Divide-by-zero during constant evaluation\n");
    } break;
    case EXP_LSHIFT: case EXP_RSHIFT: {
      ret->uint64 = eval_const_shift(exp->type == EXP_LSHIFT, op1_value, op2_value, target_size, is_signed, &flag);
      if(flag) warn_row_col_exit(exp->offset, "Shift length is greater than integer size\n");
    } break;
    case EXP_LESS: case EXP_GREATER: case EXP_LEQ: case EXP_GEQ: case EXP_EQ: case EXP_NEQ: {
      ret->type = type_getint(BASETYPE_INT);
      ret->int32 = eval_const_cmp(exp->type, op1_value, op2_value, target_size, is_signed);
    } break;
    case EXP_BIT_AND: case EXP_BIT_OR: case EXP_BIT_XOR: {  
      ret->uint64 = eval_const_bitwise(exp->type, op1_value, op2_value, target_size);
    } break;
    default: assert(0); break; // If there is an unknown op it must not pass the previous switch stmt
  }
  return ret;
}

value_t *eval_const_to_type(type_cxt_t *cxt, token_t *exp, type_t *type, int cast_type) {
  assert(cast_type == TYPE_CAST_IMPLICIT || cast_type == TYPE_CAST_EXPLICIT);
  value_t *value = eval_const_exp(cxt, exp);
  int cast_action = type_cast(type, value->type, cast_type, exp->offset);
  int sign_ext = cast_action == TYPE_CAST_SIGN_EXT;
  value->uint64 = eval_const_adjust_size(value, type->size, value->type->size, sign_ext);
  value->type = type; // Assign the new type
  return value;
}