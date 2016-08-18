
#pragma once

#include "lex.h"
#include "token.h"

/*
 * class SyntaxNode - Represents grammar elements
 *
 * This class contains a token pointer, to which to ciaims ownership
 * Also there is a vector holding it children nodes
 */
class SyntaxNode {
 private:
  // This holds the type and optionally the payload of
  // the syntax node
  //
  // Note that in most of the cases where there is no data associated
  // with the token node, we waste 8 bytes just to store nullptr
  Token *token_p;
  
  // This vector holds its children nodes in left-to-right
  // order
  std::vector<SyntaxNode *> child_list;
 public:

  /*
   * Constructor
   *
   * Node that once the token node is assigned to a syntax node, the
   * lifetime of the token node is synchronized with the syntax node
   */
  SyntaxNode(Token *p_token_p) :
    token_p{p_token_p},
    child_list{}
  {}
  
  /*
   * Destructor - Recursively removes all children nodes
   */
  ~SyntaxNode() {
    delete token_p;
    
    // Recursively remove all nodes below
    for(SyntaxNode *t : child_list) {
      delete t;
    }
    
    return;
  }
  
  /*
   * GetType() - Returns the type of the underlying token
   */
  inline TokenType GetType() const {
    return token_p->GetType();
  }
};

/*
 * class ExpressionContext - This is the context for parsing expressions
 *
 * We use a stack-based approach to parse expressions. Since we might evaluate
 * expressions recursively (e.g. function arguments are not connected by any
 * operator, so we need to parse them separately), we might need few contexts
 * be presense together
 */
class ExpressionContext {
 private:
  std::stack<SyntaxNode *> op_stack;
  std::stack<SyntaxNode *> value_stack;
  
  // If the last modified stack is op stack then this is set to true
  // This is controlled automatically by the push and pop functions
  bool op_stack_last_modified;
  
 public:
  /*
   * Constructor - Initialize member variables
   *
   * We need to initialize the boolean variable to true since when both
   * stacks are empty, if we see an operator then it is definitely a
   * prefix operator (including prefix "(")
   */
  ExpressionContext() :
    op_stack{},
    value_stack{},
    op_stack_last_modified{true}
  {}
  
  /*
   * PushOp() - Push an operator syntax node into the stack
   */
  inline void PushOp(SyntaxNode *node_p) {
    op_stack.push(node_p);
    op_stack_last_modified = true;
    
    return;
  }
  
  /*
   * PopOp() - Pops an operator from the stack
   *
   * If the stack is empty just return nullptr and it should be
   * treated as an error
   */
  inline SyntaxNode *PopOp() {
    if(op_stack.size() == 0UL) {
      return nullptr;
    }
    
    SyntaxNode *node_p = op_stack.top();
    op_stack.pop();
    
    return node_p;
  }
  
  /*
   * PushValue() - Push a syntax node into value stack
   *
   * This function will set op_stack_last_modified as false as a by-product
   */
  inline void PushValue(SyntaxNode *node_p) {
    value_stack.push(node_p);
    op_stack_last_modified = false;
    
    return;
  }
  
  /*
   * PopValue() - Returns a value SyntaxNode
   *
   * If the value stack is empty just return nullptr. This is almost an error
   */
  inline SyntaxNode *PopValue() {
    if(value_stack.size() == 0UL) {
      return nullptr;
    }
    
    SyntaxNode *node_p = value_stack.top();
    value_stack.top();
    
    return node_p;
  }
  
  /*
   * IsPrefix() - Returns whether the op stack is the last stack we have
   *              modified
   *
   * This is equivalent to whether the coming operator is prefix operator
   * or not since if op stack is the last stack modified then we have seen
   * an op and is expceting a value or a prefix op
   */
  bool IsPrefix() const {
    return op_stack_last_modified;
  }
};

/*
 * class SyntaxAnalyzer - Analyzes syntax and builds syntax tree
 */
class SyntaxAnalyzer {
 private:
  // This holds raw data and also acts as a tokenizer
  // The syntax analyzer does not have ownership of this pointer
  // and the source file should be destroyed separately
  SourceFile *source_p;

 public:
   
  /*
   * Constructor
   */
  SyntaxAnalyzer(SourceFile *p_source_p) :
    source_p{p_source_p}
  {}
  
  /*
   * Destructor
   */
  ~SyntaxAnalyzer() {}
  
  /*
   * Deleted function
   */
  SyntaxAnalyzer(const SyntaxAnalyzer &) = delete;
  SyntaxAnalyzer(SyntaxAnalyzer &&) = delete;
  SyntaxAnalyzer &operator=(const SyntaxAnalyzer &) = delete;
  SyntaxAnalyzer &operator=(SyntaxAnalyzer &&) = delete;
  
  /*
   * GetExpressionNodeType() - Return the real type of a syntax node
   *
   * Since many operators are used in an implicitly overloaded manner, e.g.
   * "*" is either used as multiplication and pointer dereference. We need to
   * determine its type based on whether it is used in prefix or postfix
   * form.
   *
   * NOTE: If the op_stack_last_modified is true then we are in prefix form
   * since we have seen an operator and is expecting a operand
   */
  TokenType GetExpressionNodeType(Token *token_p,
                                  ExpressionContext *context_p) const {
    bool is_prefix = context_p->IsPrefix();
    
    switch(token_p->GetType()) {
      case TokenType::T_STAR:
        // *p; p *
        return (is_prefix == true ? \
                TokenType::T_DEREF :
                TokenType::T_MULT);
      case TokenType::T_AMPERSAND:
        // &a; a &
        return (is_prefix == true ? \
                TokenType::T_ADDR :
                TokenType::T_BITAND);
      case TokenType::T_INC:
        // ++a; a++
        return (is_prefix == true ? \
                TokenType::T_PRE_INC :
                TokenType::T_POST_INC);
      case TokenType::T_DEC:
        // --a; a--
        return (is_prefix == true ? \
                TokenType::T_PRE_DEC :
                TokenType::T_POST_DEC);
      case TokenType::T_MINUS:
        // -a; a -
        return (is_prefix == true ? \
                TokenType::T_NEG :
                TokenType::T_SUBTRACTION);
      case TokenType::T_PLUS:
        // +a; a+
        return (is_prefix == true ? \
                TokenType::T_POS :
                TokenType::T_ADDITION);
      case TokenType::T_LPAREN:
        // a(); (a)
        // TODO: Prefix parenthesis could also be type cast
        // we need to check type information for that form
        return (is_prefix == true ? \
                TokenType::T_PAREN :
                TokenType::T_FUNCCALL);
      case TokenType::T_LSPAREN:
        // Make sure it could only be on the right side
        assert(is_prefix == false);
        
        // we only have a[] form
        return TokenType::T_ARRAYSUB;
      default:
        // By default we just use the original type
        return token_p->GetType();
    }
    
    assert(false);
    return TokenType::T_INVALID;
  }
  
  /*
   * ReduceOperator() - Takes an operator out of the op stack and form
   *                    an expression with that operator and push it into
   *                    the node stack
   *
   * We need to know the number of operands of a certain operator, and
   * that kind of information is encoded inside OpInfo helper class.
   * Once the number of operands is known, we could push them into the
   * child list of the operand syntax node
   *
   * If the op stack is empty or node stack is empty then we have a parsing
   * error
   */
  void ReduceOperator(ExpressionContext *context_p) {

  }
  
  /*
   * ParseExpression() - Parse expression using a stack and return the top node
   */
  SyntaxNode *ParseExpression() {
    ExpressionContext context{};
    
    // Loops until we have seen a non-expression element
    // we need to push that token back before returning
    while(1) {
      Token *token_p = source_p->GetNextToken();
      TokenType type = token_p->GetType();

      // Get raw type, recognize whether it is prefix or postfix
      type = GetExpressionNodeType(token_p, &context);
      
      // And reset type to reflect prefix/postfix status
      token_p->SetType(type);
      
      // It must be found, including T_PAREN and T_ARRAYSUB
      auto &op_info = TokenInfo::GetOpInfo(type);
      
      // Extract precedence and associativity
      int precedence = op_info.precedence;
      EvalOrder associativity = op_info.associativity;
      int operand_num = op_info.operand_num;

      

      switch(type) {
        
      } // switch
    } // while
  }
};
