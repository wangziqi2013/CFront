
#ifndef _TOKEN_H
#define _TOKEN_H

// Types of raw tokens. 
// This enum type does not distinguish between different expression operators, i.e. both
// unary "plus" and binary "add" is T_PLUS. Extra information such as operator property 
// is derived
typedef enum {
  // Expression token types
  T_LPAREN,
  T_RPAREN,
  T_LSPAREN,
  T_RSPAREN,
  T_DOT,
  T_ARROW,
  T_INC,
  T_DEC,
  T_PLUS,
  T_MINUS,
} token_type_t;

typedef enum {
  
} op_property_t;

typedef struct {
  // Raw type of the token, dependent only on the string literal
  token_type_t type;          
  union {
    // If the token is part of an expression then this stores the property
    op_property_t property;
    // If the token is an identifier, number, char/string literal then this stores
    // the exact string
    char *str;
  };
} token_t;

#endif