
#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "stack.h"
#include "hashtable.h"

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
  T_ELLIPSIS,           // ...
  
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
  EXP_LPAREN, EXP_RPAREN,                   // ( and ) as parenthesis
  EXP_RSPAREN,                              // ]
  EXP_DOT, EXP_ARROW,                       // obj.field ptr->field
  EXP_POST_INC, EXP_PRE_INC,                // x++ x++
  EXP_POST_DEC, EXP_PRE_DEC,                // x-- --x
  EXP_PLUS, EXP_MINUS,                      // +x -x
  EXP_LOGICAL_NOT, EXP_BIT_NOT,             // !exp ~exp
  EXP_CAST,                                 // (type)
  EXP_DEREF, EXP_ADDR,                      // *ptr &x
  EXP_SIZEOF,                               // sizeof(type/name)
  EXP_MUL, EXP_DIV, EXP_MOD,                // binary * / %
  EXP_ADD, EXP_SUB,                         // binary + -
  EXP_LSHIFT, EXP_RSHIFT,                   // << >>
  EXP_LESS, EXP_GREATER, EXP_LEQ, EXP_GEQ,  // < > <= >=
  EXP_EQ, EXP_NEQ,                          // == !=
  EXP_BIT_AND, EXP_BIT_OR, EXP_BIT_XOR,     // binary & | ^
  EXP_LOGICAL_AND, EXP_LOGICAL_OR,          // && ||
  EXP_COND, EXP_COLON,                      // ? :
  EXP_ASSIGN,                               // =
  EXP_ADD_ASSIGN, EXP_SUB_ASSIGN,           // += -=
  EXP_MUL_ASSIGN, EXP_DIV_ASSIGN, EXP_MOD_ASSIGN, // *= /= %=
  EXP_AND_ASSIGN, EXP_OR_ASSIGN, EXP_XOR_ASSIGN,  // &= |= ^=
  EXP_LSHIFT_ASSIGN, EXP_RSHIFT_ASSIGN,     // <<= >>=
  EXP_COMMA,                                // ,
  EXP_END,
  // Internal nodes
  T_UDEF,                         // User-defined type using type-def; they are not identifiers
  T_DECL, T_BASETYPE,             // Root node of a declaration
  T_,                             // Placeholder
  T_COMP_DECL,                    // structure or union declaration line, can contain one base and multiple declarator
  T_COMP_FIELD,                   // Single field declaration; Contains a DECL and optional number for bitfield
  T_LBL_STMT,
  T_EXP_STMT,
  T_COMP_STMT,
  T_INIT_LIST,
  T_STMT_LIST,                    // Contains a list of statements
  T_DECL_STMT_LIST,               // Contains a list of entries
  T_DECL_STMT_ENTRY,              // Contains a base type and a list of vars
  T_DECL_STMT_VAR,                // Contains a decl and optional initializer expression/list
  T_ROOT,
  T_GLOBAL_FUNC,                  // Global function definition
  T_GLOBAL_DECL_ENTRY,            // Global declaration (same layout as T_DECL_STMT_ENTRY)
  T_GLOBAL_DECL_VAR,              // Single entry that contains name and initializer

  T_ILLEGAL = 10000,    // Mark a return value
} token_type_t;

// Declaration properties, see below
typedef uint32_t decl_prop_t;
#define DECL_NULL          0x00000000
#define DECL_INVALID       0xFFFFFFFF // Naturally incompatible with all
// Type specifier bit mask (bit 4, 5, 6, 7)
#define DECL_TYPE_MASK     0x000000F0
#define DECL_CHAR     0x00000010
#define DECL_SHORT    0x00000020
#define DECL_INT      0x00000030
#define DECL_LONG     0x00000040
#define DECL_ENUM     0x00000050
#define DECL_STRUCT   0x00000060
#define DECL_UNION    0x00000070
#define DECL_UDEF     0x00000080 // User defined using typedef
#define DECL_FLOAT    0x00000090
#define DECL_DOUBLE   0x000000A0
#define DECL_VOID     0x000000B0
#define DECL_UNSIGNED 0x000000C0
#define DECL_SIGNED   0x000000D0
// Storage class bit mask (bit 8, 9, 10, 11)
#define DECL_STGCLS_MASK      0x00000F00
#define DECL_TYPEDEF   0x00000100
#define DECL_EXTERN    0x00000200
#define DECL_AUTO      0x00000300
#define DECL_REGISTER  0x00000400
#define DECL_STATIC    0x00000500
// Type qualifier bit mask (bit 12, 13); Note that these two are compatible
#define DECL_QUAL_MASK     0x00003000
#define DECL_VOLATILE_MASK 0x00001000
#define DECL_CONST_MASK    0x00002000
// All together, if any of these bits are present, then it is a declaration keyword
#define DECL_MASK (DECL_TYPE_MASK | DECL_STGCLS_MASK | DECL_QUAL_MASK)
// The following defines complete set of supported types (bit 16 - 23)
#define BASETYPE_MASK       0x00FF0000
#define BASETYPE_NONE       0x00000000
#define BASETYPE_CHAR       0X00010000
#define BASETYPE_SHORT      0X00020000
#define BASETYPE_INT        0X00030000
#define BASETYPE_LONG       0X00040000
#define BASETYPE_UCHAR      0X00050000
#define BASETYPE_USHORT     0X00060000
#define BASETYPE_UINT       0X00070000
#define BASETYPE_ULONG      0X00080000
#define BASETYPE_LLONG      0x00090000
#define BASETYPE_ULLONG     0x000A0000
#define BASETYPE_FLOAT      0x000B0000
#define BASETYPE_DOUBLE     0x000C0000
#define BASETYPE_LDOUBLE    0x000D0000
#define BASETYPE_STRUCT     0x000E0000
#define BASETYPE_UNION      0x000F0000
#define BASETYPE_ENUM       0x00100000
#define BASETYPE_UDEF       0x00110000
#define BASETYPE_VOID       0x00120000
#define BASETYPE_GET(decl_prop) (decl_prop & 0x00FF0000)
#define BASETYPE_SET(token, type) (token->decl_prop |= (type & 0x00FF0000))

typedef struct token_t {
  token_type_t type;
  char *str;
  struct token_t *child;
  union {
    struct token_t *sibling; // If token is in AST then use child-sibling representation
    struct token_t *next;    // If token is in pushbacks queue then form a circular queue
  };
  struct token *parent;      // Empty for root node
  char *offset;              // The offset in source file, for debugging
  decl_prop_t decl_prop;     // Property if the kwd is part of declaration; Set when a kwd is found
} token_t;

typedef struct {
  hashtable_t *udef_types;   // Auto detected when lexing T_IDENT
  token_t *pushbacks;        // Unused look-ahead symbols
  int pb_num;                // Number of pushbacks
  int ignore_pb;             // Whether to ignore pushbacked tokens
  char *s;                   // Current read position
  char *begin;               // Begin of the current text
} token_cxt_t;

typedef enum {
  ASSOC_LR, ASSOC_RL,
} assoc_t;

extern const char *keywords[32];
extern uint32_t kwd_props[32];
extern int precedences[51];

token_cxt_t *token_cxt_init(char *input);
void token_cxt_free(token_cxt_t *cxt);
void token_add_utype(token_cxt_t *cxt, token_t *token);
int token_isutype(token_cxt_t *cxt, token_t *token);
int token_decl_compatible(token_t *dest, token_t *src);
int token_decl_apply(token_t *dest, token_t *src);
char *token_decl_print(decl_prop_t decl_prop);
void token_get_property(token_type_t type, int *preced, assoc_t *assoc);
int token_get_num_operand(token_type_t type);
token_type_t token_get_keyword_type(const char *s);
const char *token_typestr(token_type_t type);
const char *token_symstr(token_type_t type);
char *token_get_op(char *s, token_t *token);
void token_copy_literal(token_t *token, const char *begin, const char *end);
void token_free(token_t *token);
token_t *token_alloc();
token_t *token_alloc_type(token_type_t type);
token_t *token_get_empty();
char *token_get_ident(token_cxt_t *cxt, char *s, token_t *token);
char *token_get_int(char *s, token_t *token);
char *token_get_str(char *s, token_t *token, char closing);
token_t *token_get_next(token_cxt_t *cxt);
int token_consume_type(token_cxt_t *cxt, token_type_t type);
void token_pushback(token_cxt_t *cxt, token_t *token);
token_t *token_lookahead(token_cxt_t *cxt, int num);
token_t *token_lookahead_notnull(token_cxt_t *cxt, int num);

#endif