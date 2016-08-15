
#include "token.h"

// Definition of the static class member
const TokenInfo::keyword_map_type TokenInfo::keyword_map {
  {
  	TokenInfo::keyword_map_value_type{"auto", TokenType::T_AUTO},
  	TokenInfo::keyword_map_value_type{"case", TokenType::T_CASE},
  	TokenInfo::keyword_map_value_type{"const", TokenType::T_CONST},
  	TokenInfo::keyword_map_value_type{"default", TokenType::T_DEFAULT},
  	TokenInfo::keyword_map_value_type{"double", TokenType::T_DOUBLE},
  	TokenInfo::keyword_map_value_type{"enum", TokenType::T_ENUM},
  	TokenInfo::keyword_map_value_type{"float", TokenType::T_FLOAT},
  	TokenInfo::keyword_map_value_type{"goto", TokenType::T_GOTO},
  	TokenInfo::keyword_map_value_type{"int", TokenType::T_INT},
  	TokenInfo::keyword_map_value_type{"register", TokenType::T_REGISTER},
  	TokenInfo::keyword_map_value_type{"short", TokenType::T_SHORT},
  	TokenInfo::keyword_map_value_type{"sizeof", TokenType::T_SIZEOF},
  	TokenInfo::keyword_map_value_type{"struct", TokenType::T_STRUCT},
  	TokenInfo::keyword_map_value_type{"typedef", TokenType::T_TYPEDEF},
  	TokenInfo::keyword_map_value_type{"unsigned", TokenType::T_UNSIGNED},
  	TokenInfo::keyword_map_value_type{"volatile", TokenType::T_VOLATILE},
  	TokenInfo::keyword_map_value_type{"break", TokenType::T_BREAK},
  	TokenInfo::keyword_map_value_type{"char", TokenType::T_CHAR},
  	TokenInfo::keyword_map_value_type{"continue", TokenType::T_CONTINUE},
  	TokenInfo::keyword_map_value_type{"do", TokenType::T_DO},
  	TokenInfo::keyword_map_value_type{"else", TokenType::T_ELSE},
  	TokenInfo::keyword_map_value_type{"extern", TokenType::T_EXTERN},
  	TokenInfo::keyword_map_value_type{"for", TokenType::T_FOR},
  	TokenInfo::keyword_map_value_type{"if", TokenType::T_IF},
  	TokenInfo::keyword_map_value_type{"long", TokenType::T_LONG},
  	TokenInfo::keyword_map_value_type{"return", TokenType::T_RETURN},
  	TokenInfo::keyword_map_value_type{"signed", TokenType::T_SIGNED},
  	TokenInfo::keyword_map_value_type{"while", TokenType::T_WHILE},
  	TokenInfo::keyword_map_value_type{"switch", TokenType::T_SWITCH},
  	TokenInfo::keyword_map_value_type{"union", TokenType::T_UNION},
  	TokenInfo::keyword_map_value_type{"void", TokenType::T_VOID},
  	TokenInfo::keyword_map_value_type{"static", TokenType::T_STATIC},
  }
};
