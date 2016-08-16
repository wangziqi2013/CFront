
#pragma once

#include <utility>
#include <unordered_map>
#include <string>
#include <functional>

enum class TokenType {
  // The following is keyword types
  T_AUTO = 0,
  
  T_BREAK,
  
  T_CASE,
	T_CHAR,
	T_CONST,
	T_CONTINUE,
	
	T_DEFAULT,
	T_DO,
	T_DOUBLE,
	
	T_ELSE,
	T_ENUM,
	T_EXTERN,
	
	T_FLOAT,
	T_FOR,
	
	T_GOTO,
	
	T_IF,
	T_INT,
	
	T_LONG,
	
	T_REGISTER,
	T_RETURN,
	
	T_SHORT,
	T_SIGNED,
	// T_SIZEOF -> This is part of the expression system
	T_STATIC,
	T_STRUCT,
	T_SWITCH,
	
	T_TYPEDEF,
	
	T_UNION,
	T_UNSIGNED,
	
	T_VOID,
	T_VOLATILE,
	
	T_WHILE,
	
	// The following are types with data
	
	T_IDENT,        // Identifier
  T_INT_CONST,    // Integer constant
  T_STRING_CONST, // String literal

	// The following are primitive operator types
	
	T_INC = 100,
	T_DEC,
	T_LPAREN,
	T_RPAREN,
  T_RSPAREN,
  T_LSPAREN,
  T_RCPAREN,
  T_LCPAREN,
  T_DOT,
  T_ARROW,
  T_PLUS = 110,
  T_MINUS,
  T_NOT,
  T_BITNOT,
  T_STAR,
  T_AMPERSAND,
  T_DIV,
  T_MOD,
  T_LSHIFT,
  T_RSHIFT,
  T_LESS = 120,
  T_LESSEQ,
  T_GREATER,
  T_GREATEREQ,
  T_EQ,
  T_NOTEQ,
  T_BITXOR,
  T_BITOR,
  T_AND,
  T_OR,
  T_QMARK = 130,
  T_COMMA,
  T_COLON,
  T_SEMICOLON,
  T_SQUOTE,
  T_DQUOTE,
  T_ASSIGN,
  T_PLUS_ASSIGN,
  T_MINUS_ASSIGN,
  T_STAR_ASSIGN,
  T_DIV_ASSIGN = 140,
  T_MOD_ASSIGN,
  T_LSHIFT_ASSIGN,
  T_RSHIFT_ASSIGN,
  T_AMPERSAND_ASSIGN,
  T_BITXOR_ASSIGN,
  T_BITOR_ASSIGN,
  T_SIZEOF = 147,
  
  // The following are operator types for overloading in C
  // e.g. ++ and -- have pre- and post-fix form
  
  // ++
  T_POST_INC = 200,
  T_PRE_INC = 201,
  
  // --
  T_POST_DEC = 202,
  T_PRE_DEC = 203,

  // *
  T_MULT = 204,
  T_DEREF = 205,

  // &
  T_ADDR = 206,
  T_BITAND = 207,
  
  // -
  T_NEG = 208,
  T_SUBTRACTION = 209,
  
  // +
  T_POS = 210,
  T_ADDITION = 211,
  
  // ()
  T_FUNCCALL = 212,
  // []
  T_ARRAYSUB = 213,
  // (TYPE)
  T_TYPECAST = 214,
  
};

// This defines the evaluation order of operators in the same
// precedence level
// i.e. associativity
enum class EvalOrder {
  LEFT_TO_RIGHT = 0,
  RIGHT_TO_LEFT,
};

/*
 * struct TokenTypeHasher - Hash function for enum class
 */
struct TokenTypeHasher {
  inline size_t operator()(const TokenType &tt) const {
    return static_cast<size_t>(tt);
  }
};

/*
 * struct TokenTypeEq - Comparison function for enum class
 */
struct TokenTypeEq {
  inline bool operator()(const TokenType &tt1, const TokenType &tt2) const {
    return static_cast<int>(tt1) == static_cast<int>(tt2);
  }
};

class TokenInfo {
  using keyword_map_value_type = std::pair<std::string, TokenType>;
  using keyword_map_type = std::unordered_map<std::string, TokenType>;

  // The value type used in operator map
  using op_map_value_type = \
    std::pair<TokenType, std::pair<int, EvalOrder>>;
    
  using op_map_type = \
    std::unordered_map<TokenType,
                       std::pair<int, EvalOrder>,
                       TokenTypeHasher,
                       TokenTypeEq>;

  static const keyword_map_type keyword_map;
  static const op_map_type op_map;
};
