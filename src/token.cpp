
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
    // precedence = 100
    
    // "(" is also considered as an operator of the lowest precedence
    // since any operator could not cause a reduce
    TokenInfo::op_map_value_type{TokenType::T_PAREN,
                                 OpInfo{100, -1, EvalOrder::RIGHT_TO_LEFT}},
    
    // precedence = 2
  	TokenInfo::op_map_value_type{TokenType::T_POST_INC,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_POST_DEC,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT}},
    // Its two arguments are function name and param list
    // we treat param list as a single syntax node to avoid argument number
    // problem (though the syntax node does not have a value)
    TokenInfo::op_map_value_type{TokenType::T_FUNCCALL,
                                 OpInfo{2, 2, EvalOrder::LEFT_TO_RIGHT}},
    // ARRAYSUB takes 2 operands: 1 array expression, 1 index expression
    TokenInfo::op_map_value_type{TokenType::T_ARRAYSUB,
                                 OpInfo{2, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_DOT,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_ARROW,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT}},
  	
  	// precedence = 3
  	TokenInfo::op_map_value_type{TokenType::T_PRE_INC,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_PRE_DEC,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_POS,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_NEG,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_NOT,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_BITNOT,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_TYPECAST,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_DEREF,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_ADDR,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_SIZEOF,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT}},
                                 
    // Precedence 5
    TokenInfo::op_map_value_type{TokenType::T_MULT,
                                 OpInfo{5, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_DIV,
                                 OpInfo{5, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_MOD,
                                 OpInfo{5, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 6
    TokenInfo::op_map_value_type{TokenType::T_ADDITION,
                                 OpInfo{6, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_SUBTRACTION,
                                 OpInfo{6, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 7
    TokenInfo::op_map_value_type{TokenType::T_LSHIFT,
                                 OpInfo{7, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_RSHIFT,
                                 OpInfo{7, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 8
    TokenInfo::op_map_value_type{TokenType::T_LESS,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_LESSEQ,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_GREATER,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_GREATEREQ,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 9
    TokenInfo::op_map_value_type{TokenType::T_EQ,
                                 OpInfo{9, 2, EvalOrder::LEFT_TO_RIGHT}},
    TokenInfo::op_map_value_type{TokenType::T_NOTEQ,
                                 OpInfo{9, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 10
    TokenInfo::op_map_value_type{TokenType::T_BITAND,
                                 OpInfo{10, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 11
    TokenInfo::op_map_value_type{TokenType::T_BITXOR,
                                 OpInfo{11, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 12
    TokenInfo::op_map_value_type{TokenType::T_BITOR,
                                 OpInfo{12, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 13
    TokenInfo::op_map_value_type{TokenType::T_AND,
                                 OpInfo{13, 2, EvalOrder::LEFT_TO_RIGHT}},
    
    // Precedence 14
    TokenInfo::op_map_value_type{TokenType::T_OR,
                                 OpInfo{14, 2, EvalOrder::LEFT_TO_RIGHT}},
                                 
    // Precedence 16
    TokenInfo::op_map_value_type{TokenType::T_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_PLUS_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_MINUS_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_STAR_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_DIV_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_MOD_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_LSHIFT_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_RSHIFT_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_AMPERSAND_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_BITXOR_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
    TokenInfo::op_map_value_type{TokenType::T_BITOR_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT}},
  }
};
