
#ifndef _EVAL_H
#define _EVAL_H

#include "token.h"
#include "ast.h"

typedef struct {
  int sign;      // 0 means unsigned, 1 means signed
  int size;      // Number of bytes
} int_t;

extern int_t ints[];

char *eval_const_atoi_maxbite(char *s, int base, token_t *token, int *ret); // Take a maximum bite and return the next to read
int eval_const_atoi(char *s, int base, token_t *token, int max_char); // Given a string and base convert to integer
char eval_const_char_token(token_t *token); // Convert char literal into a char type var
// Evaluating const expression using native int types (or convert other types to int)
int eval_const_int_token(token_t *token); // Works for integer immediate values, signed integer only; error if type incorrect
int eval_const_int(token_t *token); // Integer expression, no type info, only used for array range expression

#endif