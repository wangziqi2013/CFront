
#ifndef _EVAL_H
#define _EVAL_H

#include "token.h"
#include "ast.h"
#include "type.h"

#define EVAL_MAX(a, b) (a > b ? a : b)
#define EVAL_MIN(a, b) (a < b ? a : b)

// Used as the parameter to eval_const_atoi()
#define ATOI_NO_CHECK_END   0
#define ATOI_CHECK_END      1  // Do not report error if there is still char after the int literal
#define ATOI_NO_MAX_CHAR    0  // For \xhh \ooo we only eat 2 and 3 chars respectively

#define EVAL_MAX_CONST_SIZE 8  // We only support evaluating constants smaller than this size

uint64_t eval_int_masks[9];

uint64_t eval_const_get_mask(int op);
uint64_t eval_const_get_sign_mask(int op);
uint64_t eval_const_adjust_size(value_t *value, int to, int from, int is_signed);
uint64_t eval_const_add(value_t *op1, value_t *op2, int size, int is_signed, int *overflow);
uint64_t eval_const_sub(value_t *op1, value_t *op2, int size, int is_signed, int *overflow);
uint64_t eval_const_neg(value_t *value, int size);
uint64_t eval_const_mul(value_t *op1, value_t *op2, int size, int is_signed, int *overflow);
uint64_t eval_const_div_mod(int is_div, value_t *op1, value_t *op2, int size, int is_signed, int *div_zero);
uint64_t eval_const_shift(int is_left, value_t *op1, value_t *op2, int size, int is_signed, int *shift_overflow);
int eval_const_cmp(token_type_t op, value_t *op1, value_t *op2, int size, int is_signed);
uint64_t eval_const_bitwise(token_type_t op, value_t *op1, value_t *op2, int size);
uint64_t eval_const_unary(token_type_t op, value_t *value, int size);

char *eval_hex_char(char ch);
str_t *eval_print_const_str(str_t *s);

// Take a maximum bite and return the next to read
char *eval_const_atoi_maxbite(char *s, int base, token_t *token, int *ret); 
// Given a string and base convert to integer
int eval_const_atoi(char *s, int base, token_t *token, int max_char, int check_end, char **next); 
char eval_escaped_char(char escaped, token_t *token);

char eval_const_char_token(token_t *token); // Evaluates char type token to char
str_t *eval_const_str_token(token_t *token); // Evaluates string token to str_t *
// Evaluating const expression using native int types (or convert other types to int)
int eval_const_int_token(token_t *token); // Evaluates an integer type or char type token to int
int eval_const_int(type_cxt_t *cxt, token_t *token); // Integer expression, no type info, only used for array range expression

value_t *eval_const_get_int_value(type_cxt_t *cxt, token_t *token); // Evaluates int literal and returns value object

#endif