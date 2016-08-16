
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

// Operator precedence and associativity map
const TokenInfo::op_map_type TokenInfo::op_map {
  {
    // precedence = 2
  	TokenInfo::op_map_value_type{TokenType::T_POST_INC,
                                 std::make_pair(2, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_POST_DEC,
                                 std::make_pair(2, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_FUNCCALL,
                                 std::make_pair(2, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_ARRAYSUB,
                                 std::make_pair(2, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_DOT,
                                 std::make_pair(2, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_ARROW,
                                 std::make_pair(2, EvalOrder::LEFT_TO_RIGHT)},
  	
  	// precedence = 3
  	TokenInfo::op_map_value_type{TokenType::T_PRE_INC,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_PRE_DEC,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_POS,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_NEG,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_NOT,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_BITNOT,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_TYPECAST,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_DEREF,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_ADDR,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_SIZEOF,
                                 std::make_pair(3, EvalOrder::RIGHT_TO_LEFT)},
                                 
    // Precedence 5
    TokenInfo::op_map_value_type{TokenType::T_MULT,
                                 std::make_pair(5, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_DIV,
                                 std::make_pair(5, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_MOD,
                                 std::make_pair(5, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 6
    TokenInfo::op_map_value_type{TokenType::T_ADDITION,
                                 std::make_pair(6, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_SUBTRACTION,
                                 std::make_pair(6, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 7
    TokenInfo::op_map_value_type{TokenType::T_LSHIFT,
                                 std::make_pair(7, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_RSHIFT,
                                 std::make_pair(7, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 8
    TokenInfo::op_map_value_type{TokenType::T_LESS,
                                 std::make_pair(8, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_LESSEQ,
                                 std::make_pair(8, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_GREATER,
                                 std::make_pair(8, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_GREATEREQ,
                                 std::make_pair(8, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 9
    TokenInfo::op_map_value_type{TokenType::T_EQ,
                                 std::make_pair(9, EvalOrder::LEFT_TO_RIGHT)},
    TokenInfo::op_map_value_type{TokenType::T_NOTEQ,
                                 std::make_pair(9, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 10
    TokenInfo::op_map_value_type{TokenType::T_BITAND,
                                 std::make_pair(10, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 11
    TokenInfo::op_map_value_type{TokenType::T_BITXOR,
                                 std::make_pair(11, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 12
    TokenInfo::op_map_value_type{TokenType::T_BITOR,
                                 std::make_pair(12, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 13
    TokenInfo::op_map_value_type{TokenType::T_AND,
                                 std::make_pair(13, EvalOrder::LEFT_TO_RIGHT)},
    
    // Precedence 14
    TokenInfo::op_map_value_type{TokenType::T_OR,
                                 std::make_pair(14, EvalOrder::LEFT_TO_RIGHT)},
                                 
    // Precedence 16
    TokenInfo::op_map_value_type{TokenType::T_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_PLUS_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_MINUS_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_STAR_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_DIV_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_MOD_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_LSHIFT_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_RSHIFT_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_AMPERSAND_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_BITXOR_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
    TokenInfo::op_map_value_type{TokenType::T_BITOR_ASSIGN,
                                 std::make_pair(16, EvalOrder::RIGHT_TO_LEFT)},
  }
};
