
#pragma once

#include <utility>
#include <unordered_map>
#include <string>

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

	// The following are operator types
	
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
  T_PLUS,
  T_MINUS,
  T_NOT,
  T_BIT_NOT,
  T_STAR,
  T_AMPERSAND,
  T_DIV,
  T_MOD,
  T_LSHIFT,
  T_RSHIFT,
  T_LESS,
  T_LESSEQ,
  T_GREATER,
  T_GREATEREQ,
  T_EQ,
  T_NOTEQ,
  T_BITXOR,
  T_BITOR,
  T_AND,
  T_OR,
  T_QMARK,
  T_COMMA,
  T_COLON,
  T_SEMICOLON,
  T_SQUOTE,
  T_DQUOTE,
  T_ASSIGN,
  T_PLUS_ASSIGN,
  T_MINUS_ASSIGN,
  T_STAR_ASSIGN,
  T_DIV_ASSIGN,
  T_MOD_ASSIGN,
  T_LSHIFT_ASSIGN,
  T_RSHIFT_ASSIGN,
  T_AMPERSAND_ASSIGN,
  T_BITXOR_ASSIGN,
  T_BITOR_ASSIGN,
  T_SIZEOF,
};

class TokenInfo {
  using keyword_map_value_type = std::pair<std::string, TokenType>;
  using keyword_map_type = std::unordered_map<std::string, TokenType>;

  static const keyword_map_type keyword_map;
};
