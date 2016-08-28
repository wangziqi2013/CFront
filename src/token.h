
#pragma once

#include "common.h"

enum class TokenType {
  // This is a placeholder
  T_INVALID = 0,
  
  // The following is keyword types
  T_AUTO = 1,
  
  T_BREAK = 2,
  
  T_CASE = 3,
	T_CHAR,
	T_CONST,
	T_CONTINUE,
	
	T_DEFAULT = 7,
	T_DO,
	T_DOUBLE,
	
	T_ELSE = 10,
	T_ENUM,
	T_EXTERN,
	
	T_FLOAT = 13,
	T_FOR,
	
	T_GOTO = 15,
	
	T_IF = 16,
	T_INT,
	
	T_LONG = 18,
	
	T_REGISTER = 19,
	T_RETURN,
	
	T_SHORT = 21,
	T_SIGNED,
	// T_SIZEOF -> This is part of the expression system
	T_STATIC,
	T_STRUCT,
	T_SWITCH,
	
	T_TYPEDEF = 26,
	
	T_UNION = 27,
	T_UNSIGNED,
	
	T_VOID = 29,
	T_VOLATILE,
	
	T_WHILE = 31,
	
	// The following are compound types
	//
	// unsigned char, unsigned short, unsigned int and unsigned long
	// should be treated as one token instead of two tokens
	// Since they are represented as a single type rather than unsigned type
	// of a known type
  T_UCHAR = 40,
  T_USHORT,
  T_UINT,
  T_ULONG,
	
	// The following are types with data (literal token type)
	
	T_IDENT = 80,   // Identifier
  T_INT_CONST,    // Integer constant (should be of the same length as unsigned long)
  T_STRING_CONST, // String literal
  T_CHAR_CONST,   // Character literal

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
  
  // Prefix "(" is parsed as parenthesis, postfix ( is function call
  // Though prefix ( could also be type cast, that requires some
  // type checking.
  T_PAREN = 212,
  T_TYPECAST = 213,
  T_FUNCCALL = 214,
  
  // []
  T_ARRAYSUB = 215,
  
  // This one is artificial: function arguments
  // Since T_FUNCCALL only has 2 parameters, we need to group all its
  // arguments into one syntax node, otherwise the reduce functuon would not be
  // able to know how many value node should it reduce
  T_FUNCARG = 216,
  
};

// This defines the evaluation order of operators in the same
// precedence level
// i.e. associativity
enum class EvalOrder {
  LEFT_TO_RIGHT = 0,
  RIGHT_TO_LEFT,
};

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

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

/*
 * struct OpInfo - Stores information about operators including
 *                 precedence, associativity and number of operands
 */
struct OpInfo {
  // The smaller the higher
  int precedence;
  
  // -1 for parenthesis, positive number for all others
  int operand_num;
  
  // Associativity is used to resolve shift-reduce conflict
  // when the precedence is the same
  EvalOrder associativity;
  
  // Whether the operator is postfix unary operator
  // This is used to determine whether the operator after
  // this one is postfix or prefix
  bool is_postfix_unary;
};

/*
 * class TokenInfo - This is the helper class that facilitates tokenizer and
 *                   syntax analyzer
 */
class TokenInfo {
 public:
  using keyword_map_value_type = std::pair<std::string, TokenType>;
  using keyword_map_type = std::unordered_map<std::string, TokenType>;

  // The value type used in operator map
  using op_map_value_type = \
    std::pair<TokenType, OpInfo>;
    
  using op_map_type = \
    std::unordered_map<TokenType,
                       OpInfo,
                       TokenTypeHasher,
                       TokenTypeEq>;
                       
  // The next two are used in token name map that maps token to string name
  using token_name_map_value_type = std::pair<TokenType, std::string>;
  using token_name_map_type = \
    std::unordered_map<TokenType,
                       std::string,
                       TokenTypeHasher,
                       TokenTypeEq>;

  static const keyword_map_type keyword_map;
  static const op_map_type op_map;
  // This is used for debugging and error reporting
  static const token_name_map_type token_name_map;
  
  /*
   * GetOpInfo() - Return the struct of (precedence, op count, associativity)
   *               of a specific operator
   *
   * If the operator is not found then it implies the type is not part of
   * an expression, and if we are parsing an expression then probably
   * it is the end of an expression
   *
   * We return a constant pointer to the structure
   */
  static const OpInfo *GetOpInfo(TokenType type) {
    auto it = TokenInfo::op_map.find(type);
    
    // If does not find then return nullptr to indicate this
    // is not a valid operator type
    //
    // This branch is useful since
    if(it == TokenInfo::op_map.end()) {
      return nullptr;
    }
    
    return &it->second;
  }
  
  /*
   * GetTokenName() - Given a token type, return the name of that type
   *
   * The name is returned in a constant string reference form
   */
  static const std::string &GetTokenName(TokenType type) {
    auto it = TokenInfo::token_name_map.find(type);

    // If does not find then return nullptr to indicate this
    // is not a valid operator type
    //
    // This branch is useful since
    assert(it != TokenInfo::token_name_map.end());

    return it->second;
  }
  
};

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

/*
 * class Token - Main class to represent lexicon
 */
class Token {
 private:
  TokenType type;
  
  union {
    // Integer constant
    unsigned long int_const;
    
    // char constant
    char char_const;
    
    // String constant
    std::string *string_const_p;
    
    // Identifier
    std::string *ident_p;
  } data;
  
 public:

  /*
   * Constructor() - Construct a token object with corresponding type
   *
   * We choose not to set data here since it is a union
   */
  Token(TokenType p_type) :
    type{p_type} {
    // This will also clear the pointer
    data.int_const = 0;
    
    assert(data.ident_p == nullptr);
    assert(data.string_const_p == nullptr);
  }
  
  /*
   * Destructor - Frees the pointer if there is one
   *
   * The ownership of the pointer stored as identifier or string constant
   * belongs to Token object
   */
  ~Token() {
    // In both case the target is a string pointer
    // so we could just delete it without distinguishing
    // further on its type
    if(type == TokenType::T_IDENT || \
       type == TokenType::T_STRING_CONST) {
         
      // If we destruct the string in exception handler
      // then the pointer is nullptr since during construction
      // we set it to nullptr and it has not been filled with anything
      if(data.string_const_p != nullptr) {
        delete data.string_const_p;
      }
    }
    
    return;
  }
  
  /*
   * SetType() - Assigns a new type to the token
   *
   * This is necessary since we need to resolve ambiguity during parsing
   * with operator types. e.g. "*" could either be used as multiplication
   * or be used as pointer dereference operator
   */
  void SetType(TokenType p_type) {
    type = p_type;
    
    return;
  }
  
  /*
   * GetType() - Returns the type of the token
   */
  TokenType GetType() const {
    return type;
  }
  
  /*
   * SetIntConst() - Set a integer constant number to this object
   *
   * This function requires that the token type must be T_INT_CONST
   */
  void SetIntConst(unsigned long p_int_const) {
    assert(type == TokenType::T_INT_CONST);
    
    data.int_const = p_int_const;
    
    return;
  }
  
  /*
   * GetIntConst() - Returns the integer constant
   */
  unsigned long GetIntConst() const {
    assert(type == TokenType::T_INT_CONST);
    
    return data.int_const;
  }
  
  /*
   * SetCharConst() - Set a char constant to this object
   *
   * This function requires that the token must be of T_CHAR_CONST
   */
  void SetCharConst(char p_char_const) {
    assert(type == TokenType::T_CHAR_CONST);
    
    data.char_const = p_char_const;
    
    return;
  }
  
  /*
   * GetCharConst() - Returns a char constant
   */
  char GetCharConst() const {
    assert(type == TokenType::T_CHAR_CONST);
    
    return data.char_const;
  }
  
  /*
   * SetStringConst() - Set a string constant to this object
   *
   * This function requires that the token must be of T_STRING_CONST
   */
  void SetStringConst(std::string *p_string_const_p) {
    assert(type == TokenType::T_STRING_CONST);

    data.string_const_p = p_string_const_p;

    return;
  }
  
  /*
   * GetStringConst() - Returns the string pointer
   */
  std::string *GetStringConst() const {
    assert(type == TokenType::T_STRING_CONST);
    
    return data.string_const_p;
  }
  
  /*
   * SetIdentifier() - Set an identifier string to this object
   *
   * This function requires that the token must be of T_IDENT
   */
  void SetIdentifier(std::string *p_ident_p) {
    assert(type == TokenType::T_IDENT);

    data.ident_p = p_ident_p;

    return;
  }
  
  /*
   * GetIdentifier() - Returns the identifier string object
   */
  std::string *GetIdentifier() const {
    assert(type == TokenType::T_IDENT);

    return data.ident_p;
  }
  
  /*
   * ToString() - Convert the token node to string representation
   *
   * There is no trailing '\n' attached with the string
   */
  std::string ToString() const {
    const std::string &name = TokenInfo::GetTokenName(type);
    
    if(type == TokenType::T_IDENT) {
      return name + ' ' + *GetIdentifier();
    } else if(type == TokenType::T_STRING_CONST) {
      return name + ' ' + *GetStringConst();
    } else if(type == TokenType::T_INT_CONST) {
      return name + ' ' + std::to_string(GetIntConst());
    } else if(type == TokenType::T_CHAR_CONST) {
      return name + ' ' + \
             std::to_string(static_cast<int>(GetCharConst()));
    } else {
      return name;
    }
  }
};
