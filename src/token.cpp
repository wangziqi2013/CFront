
#include "token.h"

// Definition of the static class member
const KeywordMap::map_type KeywordMap::keyword_map {
  {
  	KeywordMap::value_type{"auto", TokenType::T_AUTO},
  	KeywordMap::value_type{"case", TokenType::T_CASE},
  	KeywordMap::value_type{"const", TokenType::T_CONST},
  	KeywordMap::value_type{"default", TokenType::T_DEFAULT},
  	KeywordMap::value_type{"double", TokenType::T_DOUBLE},
  	KeywordMap::value_type{"enum", TokenType::T_ENUM},
  	KeywordMap::value_type{"float", TokenType::T_FLOAT},
  	KeywordMap::value_type{"goto", TokenType::T_GOTO},
  	KeywordMap::value_type{"int", TokenType::T_INT},
  	KeywordMap::value_type{"register", TokenType::T_REGISTER},
  	KeywordMap::value_type{"short", TokenType::T_SHORT},
  	KeywordMap::value_type{"sizeof", TokenType::T_SIZEOF},
  	KeywordMap::value_type{"struct", TokenType::T_STRUCT},
  	KeywordMap::value_type{"typedef", TokenType::T_TYPEDEF},
  	KeywordMap::value_type{"unsigned", TokenType::T_UNSIGNED},
  	KeywordMap::value_type{"volatile", TokenType::T_VOLATILE},
  	KeywordMap::value_type{"break", TokenType::T_BREAK},
  	KeywordMap::value_type{"char", TokenType::T_CHAR},
  	KeywordMap::value_type{"continue", TokenType::T_CONTINUE},
  	KeywordMap::value_type{"do", TokenType::T_DO},
  	KeywordMap::value_type{"else", TokenType::T_ELSE},
  	KeywordMap::value_type{"extern", TokenType::T_EXTERN},
  	KeywordMap::value_type{"for", TokenType::T_FOR},
  	KeywordMap::value_type{"if", TokenType::T_IF},
  	KeywordMap::value_type{"long", TokenType::T_LONG},
  	KeywordMap::value_type{"return", TokenType::T_RETURN},
  	KeywordMap::value_type{"signed", TokenType::T_SIGNED},
  	KeywordMap::value_type{"while", TokenType::T_WHILE},
  	KeywordMap::value_type{"switch", TokenType::T_SWITCH},
  	KeywordMap::value_type{"union", TokenType::T_UNION},
  	KeywordMap::value_type{"void", TokenType::T_VOID},
  	KeywordMap::value_type{"static", TokenType::T_STATIC}
  }
};
