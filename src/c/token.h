
#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Types of raw tokens. 
// This enum type does not distinguish between different expression operators, i.e. both
// unary "plus" and binary "add" is T_PLUS. Extra information such as operator property 
// is derived
typedef enum {
  // Expression token types
  T_LPAREN,             // (
  T_RPAREN,             // )
  T_LSPAREN,            // [
  T_RSPAREN,            // ]
  T_DOT,                // .
  T_ARROW,              // ->
  T_INC,                // ++
  T_DEC,                // --
  T_PLUS,               // +
  T_MINUS,              // -
  T_LOGICAL_NOT,        // !
  T_BIT_NOT,            // ~
  T_STAR,               // *
  T_AND,                // &   This is both address op and bitwise and
  //T_SIZEOF,             // sizeof
  T_DIV,                // /
  T_MOD,                // %

  T_LSHIFT,             // <<
  T_RSHIFT,             // >>

  T_LESS,               // <
  T_GREATER,            // >
  T_LEQ,                // <=      
  T_GEQ,                // >=

  T_EQ,                 // ==
  T_NEQ,                // !=

  T_BIT_XOR,            // ^
  T_BIT_OR,             // |

  T_LOGICAL_AND,        // &&
  T_LOGICAL_OR,         // ||

  T_QMARK,              // ?
  T_COLON,              // :

  T_ASSIGN,             // =
  T_PLUS_ASSIGN,        // +=
  T_MINUS_ASSIGN,       // -=
  T_MUL_ASSIGN,         // *=
  T_DIV_ASSIGN,         // /=
  T_MOD_ASSIGN,         // %=
  T_LSHIFT_ASSIGN,      // <<=
  T_RSHIFT_ASSIGN,      // >>=
  T_AND_ASSIGN,         // &=
  T_OR_ASSIGN,          // |=
  T_XOR_ASSIGN,         // ^=

  T_COMMA,              // ,

  T_LCPAREN,            // {
  T_RCPAREN,            // }

  T_IDENT,

  // Add this to the index of keywords in the table
  T_KEYWORDS = 1000,
  T_AUTO, T_BREAK, T_CASE, T_CHAR, T_CONST, T_CONTINUE, T_DEFAULT, T_DO,
  T_DOUBLE, T_ELSE, T_ENUM, T_EXTERN, T_FLOAT, T_FOR, T_GOTO, T_IF,
  T_INT, T_LONG, T_REGISTER, T_RETURN, T_SHORT, T_SIGNED, T_SIZEOF, T_STATIC,
  T_STRUCT, T_SWITCH, T_TYPEDEF, T_UNION, T_UNSIGNED, T_VOID, T_VOLATILE, T_WHILE,

  T_ILLEGAL = 10000,    // Mark a return value
} token_type_t;

// Defines operator associativity
typedef enum {
  LEFT_TO_RIGHT,
  RIGHT_TO_LEFT,
} associativity_t;

typedef struct {
  // Raw type of the token, dependent only on the string literal
  token_type_t type;
  union {
    // Lower number means higher precedence
    uint8_t precedence;
    associativity_t associativity;
    // If the token is an identifier, number, char/string literal then this stores
    // the exact string
    char *str;
  };
} token_t;

extern const char *keywords[32];

char *token_get_op(char *s, token_t *token);
const char *token_typestr(token_type_t type);
const char *token_symstr(token_type_t type);
char *token_get_ident(char *s, token_t *token);
char *token_get_next(char *s, token_t *token);
token_type_t get_keyword_type(const char *s);

#endif