
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
	T_SIZEOF,
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
};

class KeywordMap {
  using value_type = std::pair<std::string, TokenType>;
  using map_type = std::unordered_map<std::string, TokenType>;

  static const map_type keyword_map;
};
