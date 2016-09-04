
#include "token.h"

using namespace wangziqi2013;
using namespace cfront;

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
    //
    // post unary is set to false since it should change is_prefix to true
    TokenInfo::op_map_value_type{TokenType::T_PAREN,
                                 OpInfo{100, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    
    // precedence = 2
  	TokenInfo::op_map_value_type{TokenType::T_POST_INC,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT, true}},
    TokenInfo::op_map_value_type{TokenType::T_POST_DEC,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT, true}},
    // Its two arguments are function name and param list
    // we treat param list as a single syntax node to avoid argument number
    // problem (though the syntax node does not have a value)
    //
    // Though it carries 2 children nodes we should set unary postfix as true
    TokenInfo::op_map_value_type{TokenType::T_FUNCCALL,
                                 OpInfo{2, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    // ARRAYSUB takes 2 operands: 1 array expression, 1 index expression
    TokenInfo::op_map_value_type{TokenType::T_ARRAYSUB,
                                 OpInfo{2, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_DOT,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_ARROW,
                                 OpInfo{2, 1, EvalOrder::LEFT_TO_RIGHT, false}},
  	
  	// precedence = 3
  	TokenInfo::op_map_value_type{TokenType::T_PRE_INC,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_PRE_DEC,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_POS,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_NEG,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_NOT,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_BITNOT,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_TYPECAST,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_DEREF,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_ADDR,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_SIZEOF,
                                 OpInfo{3, 1, EvalOrder::RIGHT_TO_LEFT, false}},
                                 
    // Precedence 5
    TokenInfo::op_map_value_type{TokenType::T_MULT,
                                 OpInfo{5, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_DIV,
                                 OpInfo{5, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_MOD,
                                 OpInfo{5, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 6
    TokenInfo::op_map_value_type{TokenType::T_ADDITION,
                                 OpInfo{6, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_SUBTRACTION,
                                 OpInfo{6, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 7
    TokenInfo::op_map_value_type{TokenType::T_LSHIFT,
                                 OpInfo{7, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_RSHIFT,
                                 OpInfo{7, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 8
    TokenInfo::op_map_value_type{TokenType::T_LESS,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_LESSEQ,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_GREATER,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_GREATEREQ,
                                 OpInfo{8, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 9
    TokenInfo::op_map_value_type{TokenType::T_EQ,
                                 OpInfo{9, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    TokenInfo::op_map_value_type{TokenType::T_NOTEQ,
                                 OpInfo{9, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 10
    TokenInfo::op_map_value_type{TokenType::T_BITAND,
                                 OpInfo{10, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 11
    TokenInfo::op_map_value_type{TokenType::T_BITXOR,
                                 OpInfo{11, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 12
    TokenInfo::op_map_value_type{TokenType::T_BITOR,
                                 OpInfo{12, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 13
    TokenInfo::op_map_value_type{TokenType::T_AND,
                                 OpInfo{13, 2, EvalOrder::LEFT_TO_RIGHT, false}},
    
    // Precedence 14
    TokenInfo::op_map_value_type{TokenType::T_OR,
                                 OpInfo{14, 2, EvalOrder::LEFT_TO_RIGHT, false}},
                                 
    // Precedence 16
    TokenInfo::op_map_value_type{TokenType::T_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_PLUS_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_MINUS_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_STAR_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_DIV_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_MOD_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_LSHIFT_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_RSHIFT_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_AMPERSAND_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_BITXOR_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
    TokenInfo::op_map_value_type{TokenType::T_BITOR_ASSIGN,
                                 OpInfo{16, 2, EvalOrder::RIGHT_TO_LEFT, false}},
  }
};

/*
 * token_name_map - A mapping from full set of TokenType enum to a string
 *                  name
 *
 * This map is majorly for outputing information for tokens
 */
const TokenInfo::token_name_map_type TokenInfo::token_name_map = {
  TokenInfo::token_name_map_value_type{TokenType::T_INVALID, "T_INVALID"},
  TokenInfo::token_name_map_value_type{TokenType::T_AUTO, "T_AUTO"},
  TokenInfo::token_name_map_value_type{TokenType::T_BREAK, "T_BREAK"},
  TokenInfo::token_name_map_value_type{TokenType::T_CASE, "T_CASE"},
	TokenInfo::token_name_map_value_type{TokenType::T_CHAR, "T_CHAR"},
	TokenInfo::token_name_map_value_type{TokenType::T_CONST, "T_CONST"},
	TokenInfo::token_name_map_value_type{TokenType::T_CONTINUE, "T_CONTINUE"},
	TokenInfo::token_name_map_value_type{TokenType::T_DEFAULT, "T_DEFAULT"},
	TokenInfo::token_name_map_value_type{TokenType::T_DO, "T_DO"},
	TokenInfo::token_name_map_value_type{TokenType::T_DOUBLE, "T_DOUBLE"},
	TokenInfo::token_name_map_value_type{TokenType::T_ELSE, "T_ELSE"},
	TokenInfo::token_name_map_value_type{TokenType::T_ENUM, "T_ENUM"},
	TokenInfo::token_name_map_value_type{TokenType::T_EXTERN, "T_EXTERN"},
  TokenInfo::token_name_map_value_type{TokenType::T_FLOAT, "T_FLOAT"},
  TokenInfo::token_name_map_value_type{TokenType::T_FOR, "T_FOR"},
  TokenInfo::token_name_map_value_type{TokenType::T_GOTO, "T_GOTO"},
  TokenInfo::token_name_map_value_type{TokenType::T_IF, "T_IF"},
  TokenInfo::token_name_map_value_type{TokenType::T_INT, "T_INT"},
  TokenInfo::token_name_map_value_type{TokenType::T_LONG, "T_LONG"},
  TokenInfo::token_name_map_value_type{TokenType::T_REGISTER, "T_REGISTER"},
  TokenInfo::token_name_map_value_type{TokenType::T_RETURN, "T_RETURN"},
  TokenInfo::token_name_map_value_type{TokenType::T_SHORT, "T_SHORT"},
  TokenInfo::token_name_map_value_type{TokenType::T_SIGNED, "T_SIGNED"},
  TokenInfo::token_name_map_value_type{TokenType::T_STATIC, "T_STATIC"},
  TokenInfo::token_name_map_value_type{TokenType::T_STRUCT, "T_STRUCT"},
  TokenInfo::token_name_map_value_type{TokenType::T_SWITCH, "T_SWITCH"},
  TokenInfo::token_name_map_value_type{TokenType::T_TYPEDEF, "T_TYPEDEF"},
  TokenInfo::token_name_map_value_type{TokenType::T_UNION, "T_UNION"},
  TokenInfo::token_name_map_value_type{TokenType::T_UNSIGNED, "T_UNSIGNED"},
  TokenInfo::token_name_map_value_type{TokenType::T_VOID, "T_VOID"},
  TokenInfo::token_name_map_value_type{TokenType::T_VOLATILE, "T_VOLATILE"},
  TokenInfo::token_name_map_value_type{TokenType::T_WHILE, "T_WHILE"},
  TokenInfo::token_name_map_value_type{TokenType::T_UCHAR, "T_UCHAR"},
  TokenInfo::token_name_map_value_type{TokenType::T_USHORT, "T_USHORT"},
  TokenInfo::token_name_map_value_type{TokenType::T_UINT, "T_UINT"},
  TokenInfo::token_name_map_value_type{TokenType::T_ULONG, "T_ULONG"},
  TokenInfo::token_name_map_value_type{TokenType::T_IDENT, "T_IDENT"},
  TokenInfo::token_name_map_value_type{TokenType::T_INT_CONST, "T_INT_CONST"},
  TokenInfo::token_name_map_value_type{TokenType::T_STRING_CONST, "T_STRING_CONST"},
  TokenInfo::token_name_map_value_type{TokenType::T_CHAR_CONST, "T_CHAR_CONST"},
  TokenInfo::token_name_map_value_type{TokenType::T_INC, "T_INC"},
  TokenInfo::token_name_map_value_type{TokenType::T_DEC, "T_DEC"},
  TokenInfo::token_name_map_value_type{TokenType::T_LPAREN, "T_LPAREN"},
  TokenInfo::token_name_map_value_type{TokenType::T_RPAREN, "T_RPAREN"},
  TokenInfo::token_name_map_value_type{TokenType::T_RSPAREN, "T_RSPAREN"},
  TokenInfo::token_name_map_value_type{TokenType::T_LSPAREN, "T_LSPAREN"},
  TokenInfo::token_name_map_value_type{TokenType::T_RCPAREN, "T_RCPAREN"},
  TokenInfo::token_name_map_value_type{TokenType::T_LCPAREN, "T_LCPAREN"},
  TokenInfo::token_name_map_value_type{TokenType::T_DOT, "T_DOT"},
  TokenInfo::token_name_map_value_type{TokenType::T_ARROW, "T_ARROW"},
  TokenInfo::token_name_map_value_type{TokenType::T_PLUS, "T_PLUS"},
  TokenInfo::token_name_map_value_type{TokenType::T_MINUS, "T_MINUS"},
  TokenInfo::token_name_map_value_type{TokenType::T_NOT, "T_NOT"},
  TokenInfo::token_name_map_value_type{TokenType::T_BITNOT, "T_BITNOT"},
  TokenInfo::token_name_map_value_type{TokenType::T_STAR, "T_STAR"},
  TokenInfo::token_name_map_value_type{TokenType::T_AMPERSAND, "T_AMPERSAND"},
  TokenInfo::token_name_map_value_type{TokenType::T_DIV, "T_DIV"},
  TokenInfo::token_name_map_value_type{TokenType::T_MOD, "T_MOD"},
  TokenInfo::token_name_map_value_type{TokenType::T_LSHIFT, "T_LSHIFT"},
  TokenInfo::token_name_map_value_type{TokenType::T_RSHIFT, "T_RSHIFT"},
  TokenInfo::token_name_map_value_type{TokenType::T_LESS, "T_LESS"},
  TokenInfo::token_name_map_value_type{TokenType::T_LESSEQ, "T_LESSEQ"},
  TokenInfo::token_name_map_value_type{TokenType::T_GREATER, "T_GREATER"},
  TokenInfo::token_name_map_value_type{TokenType::T_GREATEREQ, "T_GREATEREQ"},
  TokenInfo::token_name_map_value_type{TokenType::T_EQ, "T_EQ"},
  TokenInfo::token_name_map_value_type{TokenType::T_NOTEQ, "T_NOTEQ"},
  TokenInfo::token_name_map_value_type{TokenType::T_BITXOR, "T_BITXOR"},
  TokenInfo::token_name_map_value_type{TokenType::T_BITOR, "T_BITOR"},
  TokenInfo::token_name_map_value_type{TokenType::T_AND, "T_AND"},
  TokenInfo::token_name_map_value_type{TokenType::T_OR, "T_OR"},
  TokenInfo::token_name_map_value_type{TokenType::T_QMARK, "T_QMARK"},
  TokenInfo::token_name_map_value_type{TokenType::T_COMMA, "T_COMMA"},
  TokenInfo::token_name_map_value_type{TokenType::T_COLON, "T_COLON"},
  TokenInfo::token_name_map_value_type{TokenType::T_SEMICOLON, "T_SEMICOLON"},
  TokenInfo::token_name_map_value_type{TokenType::T_SQUOTE, "T_SQUOTE"},
  TokenInfo::token_name_map_value_type{TokenType::T_DQUOTE, "T_DQUOTE"},
  TokenInfo::token_name_map_value_type{TokenType::T_ASSIGN, "T_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_PLUS_ASSIGN, "T_PLUS_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_MINUS_ASSIGN, "T_MINUS_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_STAR_ASSIGN, "T_STAR_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_DIV_ASSIGN, "T_DIV_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_MOD_ASSIGN, "T_MOD_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_LSHIFT_ASSIGN, "T_LSHIFT_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_RSHIFT_ASSIGN, "T_RSHIFT_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_AMPERSAND_ASSIGN, "T_AMPERSAND_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_BITXOR_ASSIGN, "T_BITXOR_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_BITOR_ASSIGN, "T_BITOR_ASSIGN"},
  TokenInfo::token_name_map_value_type{TokenType::T_SIZEOF, "T_SIZEOF"},
  TokenInfo::token_name_map_value_type{TokenType::T_POST_INC, "T_POST_INC"},
  TokenInfo::token_name_map_value_type{TokenType::T_PRE_INC, "T_PRE_INC"},
  TokenInfo::token_name_map_value_type{TokenType::T_POST_DEC, "T_POST_DEC"},
  TokenInfo::token_name_map_value_type{TokenType::T_PRE_DEC, "T_PRE_DEC"},
  TokenInfo::token_name_map_value_type{TokenType::T_MULT, "T_MULT"},
  TokenInfo::token_name_map_value_type{TokenType::T_DEREF, "T_DEREF"},
  TokenInfo::token_name_map_value_type{TokenType::T_ADDR, "T_ADDR"},
  TokenInfo::token_name_map_value_type{TokenType::T_BITAND, "T_BITAND"},
  TokenInfo::token_name_map_value_type{TokenType::T_NEG, "T_NEG"},
  TokenInfo::token_name_map_value_type{TokenType::T_SUBTRACTION, "T_SUBTRACTION"},
  TokenInfo::token_name_map_value_type{TokenType::T_POS, "T_POS"},
  TokenInfo::token_name_map_value_type{TokenType::T_ADDITION, "T_ADDITION"},
  TokenInfo::token_name_map_value_type{TokenType::T_PAREN, "T_PAREN"},
  TokenInfo::token_name_map_value_type{TokenType::T_TYPECAST, "T_TYPECAST"},
  TokenInfo::token_name_map_value_type{TokenType::T_FUNCCALL, "T_FUNCCALL"},
  TokenInfo::token_name_map_value_type{TokenType::T_ARRAYSUB, "T_ARRAYSUB"},
  TokenInfo::token_name_map_value_type{TokenType::T_FUNCARG, "T_FUNCARG"},
};

/*
 * builtin_type_set - A set of token types that represent built in types
 */
const std::unordered_set<TokenType, TokenTypeHasher, TokenTypeEq>
TokenInfo::builtin_type_set = {
  TokenType::T_CHAR, TokenType::T_UCHAR,
  TokenType::T_SHORT, TokenType::T_USHORT,
  TokenType::T_INT, TokenType::T_UINT,
  TokenType::T_LONG, TokenType::T_ULONG,
};

