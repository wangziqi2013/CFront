
#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

// Types of raw tokens. 
// This enum type does not distinguish between different expression operators, i.e. both
// unary "plus" and binary "add" is T_PLUS. Extra information such as operator property 
// is derived
typedef enum {
  // Expression token types
  T_OP_BEGIN = 0,
  T_LPAREN = 0, T_RPAREN, T_LSPAREN, T_RSPAREN,       // ( ) [ ]
  T_DOT, T_ARROW,                                     // . ->
  T_INC, T_DEC, T_PLUS, T_MINUS,                      // ++ -- + -
  T_LOGICAL_NOT = 10, T_BIT_NOT,                      // ! ~
  T_STAR, T_AND,                                      // * &
  T_DIV, T_MOD,                                       // / %
  T_LSHIFT, T_RSHIFT,                                 // << >>

  T_LESS, T_GREATER, T_LEQ = 20, T_GEQ, T_EQ, T_NEQ,  // < > <= >= == !=
  T_BIT_XOR, T_BIT_OR,                                // ^ |
  T_LOGICAL_AND, T_LOGICAL_OR,                        // && ||
  T_QMARK, T_COLON,                                   // ? :
  T_ASSIGN = 30,                                      // =
  T_PLUS_ASSIGN, T_MINUS_ASSIGN, T_MUL_ASSIGN,        // = += -= *=
  T_DIV_ASSIGN, T_MOD_ASSIGN,                         // /= %=
  T_LSHIFT_ASSIGN, T_RSHIFT_ASSIGN,                   // <<= >>=
  T_AND_ASSIGN, T_OR_ASSIGN, T_XOR_ASSIGN = 40,       // &= |= ^=
  T_COMMA,                                            // ,
  T_OP_END,

  T_LCPAREN,            // {
  T_RCPAREN,            // }
  T_SEMICOLON,          // ;
  
  // Literal types (i.e. primary expressions)
  T_LITERALS_BEGIN = 200,
  T_DEC_INT_CONST = 200, T_HEX_INT_CONST, T_OCT_INT_CONST,
  T_CHAR_CONST, T_STR_CONST,
  T_FLOAT_CONST,
  T_IDENT,
  T_LITERALS_END,

  // Add this to the index of keywords in the table
  T_KEYWORDS_BEGIN = 1000,
  T_AUTO = 1000, T_BREAK, T_CASE, T_CHAR, T_CONST, T_CONTINUE, T_DEFAULT, T_DO,
  T_DOUBLE, T_ELSE, T_ENUM, T_EXTERN, T_FLOAT, T_FOR, T_GOTO, T_IF,
  T_INT, T_LONG, T_REGISTER, T_RETURN, T_SHORT, T_SIGNED, T_SIZEOF, T_STATIC,
  T_STRUCT, T_SWITCH, T_TYPEDEF, T_UNION, T_UNSIGNED, T_VOID, T_VOLATILE, T_WHILE,
  T_KEYWORDS_END,

  // AST type used within an expression (51 elements)
  EXP_BEGIN = 2000,
  EXP_FUNC_CALL = 2000, EXP_ARRAY_SUB,      // func() array[]
  EXP_LPAREN, EXP_RPAREN,                // ( and ) as parenthesis
  EXP_RSPAREN,                // ]
  EXP_DOT, EXP_ARROW,                   // obj.field ptr->field
                    
  EXP_POST_INC,               // x++
  EXP_PRE_INC,                // ++x
  EXP_POST_DEC,               // x--
  EXP_PRE_DEC,                // --x
  EXP_PLUS,                   // +x
  EXP_MINUS,                  // -x
  EXP_LOGICAL_NOT,            // !exp
  EXP_BIT_NOT,                // ~exp
  EXP_CAST,                   // (type)
  EXP_DEREF,                  // *ptr
  EXP_ADDR,                   // &x
  EXP_SIZEOF,                 // sizeof(type/name)
  EXP_MUL, EXP_DIV, EXP_MOD,  // binary * / %
  EXP_ADD, EXP_SUB,           // binary + -
  EXP_LSHIFT, EXP_RSHIFT,     // << >>
  EXP_LESS, EXP_GREATER, EXP_LEQ, EXP_GEQ, // < > <= >=
  EXP_EQ, EXP_NEQ,            // == !=
  EXP_BIT_AND, EXP_BIT_OR, EXP_BIT_XOR,    // binary & | ^
  EXP_LOGICAL_AND, EXP_LOGICAL_OR,         // && ||
  EXP_COND,                                // ? :
  EXP_COLON,                               // Used in ? : expression
  EXP_ASSIGN,                              // =
  EXP_ADD_ASSIGN, EXP_SUB_ASSIGN,          // += -=
  EXP_MUL_ASSIGN, EXP_DIV_ASSIGN, EXP_MOD_ASSIGN, // *= /= %=
  EXP_AND_ASSIGN, EXP_OR_ASSIGN, EXP_XOR_ASSIGN,  // &= |= ^=
  EXP_LSHIFT_ASSIGN, EXP_RSHIFT_ASSIGN,    // <<= >>=
  EXP_COMMA,                               // binary ,
  EXP_END,

  T_UDEF_TYPE,   // User-defined type using type-def; they are not identifiers
  T_DECL,        // Root node of a declaration

  T_ILLEGAL = 10000,    // Mark a return value
  T_STOP,               // Used to instruct the parser to stop
} token_type_t;

// Keyword property flags (bit 0, 1, 2, 3)
#define KWD_PROP_ISDECL    0x00000001
#define KWD_PROP_STGCLS    0x00000002  // typedef extern auto register static
#define KWD_PROP_TYPE      0x00000004  // void char short int long float double signed unsigned struct union (also typedef'ed name)
#define KWD_PROP_QUAL      0x00000008  // const volatile
// Type related bit mask (bit 4, 5, 6, 7)
#define DECL_TYPE_MASK     0x000000F0
#define DECL_TYPE_NONE     0x00000000
#define DECL_TYPE_CHAR     0x00000010
#define DECL_TYPE_SHORT    0x00000020
#define DECL_TYPE_INT      0x00000030
#define DECL_TYPE_LONG     0x00000040
#define DECL_TYPE_UCHAR    0x00000050
#define DECL_TYPE_USHORT   0x00000060
#define DECL_TYPE_UINT     0x00000070
#define DECL_TYPE_ULONG    0x00000080
#define DECL_TYPE_ENUM     0x00000090
#define DECL_TYPE_STRUCT   0x000000A0
#define DECL_TYPE_UNION    0x000000B0
#define DECL_TYPE_TYPEDEF  0x000000C0
#define DECL_TYPE_FLOAT    0x000000D0
#define DECL_TYPE_DOUBLE   0x000000E0
#define DECL_TYPE_VOID     0x000000F0
// Storage class bit mask (bit 8, 9, 10, 11)
#define DECL_STG_MASK      0x00000F00
#define DECL_STG_NONE      0x00000000
#define DECL_STG_TYPEDEF   0x00000100
#define DECL_STG_EXTERN    0x00000200
#define DECL_STG_AUTO      0x00000300
#define DECL_STG_REGISTER  0x00000400
#define DECL_STG_STATIC    0x00000500
// Type qualifier bit mask (bit 12, 13)
#define DECL_QUAL_MASK     0x00003000
#define DECL_QUAL_VOLATILE 0x00001000
#define DECL_STG_TYPEDEF   0x00002000

typedef struct token_t {
  token_type_t type;
  char *str;
  struct token_t *child;
  struct token_t *sibling;
  // The offset in source file, for debugging
  char *offset;
} token_t;

typedef enum {
  ASSOC_LR, ASSOC_RL,
} assoc_t;

extern const char *keywords[32];
extern uint32_t kwd_props[32];
extern int precedences[51];

int kwd_isdecl(token_type_t type);
void token_get_property(token_type_t type, int *preced, assoc_t *assoc);
int token_get_num_operand(token_type_t type);
token_type_t token_get_keyword_type(const char *s);
const char *token_typestr(token_type_t type);
const char *token_symstr(token_type_t type);
char *token_get_op(char *s, token_t *token);
void token_copy_literal(token_t *token, const char *begin, const char *end);
void token_free_literal(token_t *token);
void token_free(token_t *token);
token_t *token_alloc();
char *token_get_ident(char *s, token_t *token);
char *token_get_int(char *s, token_t *token);
char *token_get_str(char *s, token_t *token, char closing);
char *token_get_next(char *s, token_t *token);

#endif