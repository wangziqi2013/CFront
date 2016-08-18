
#pragma once

#include <algorithm>

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
  
  /*
   * GetToken() - Returns the token node embedded inside the syntax node
   */
  inline Token *GetToken() const {
    return token_p;
  }
  
  /*
   * GetChildList() - Return the child list reference
   */
  inline std::vector<SyntaxNode *> &GetChildList() {
    return child_list;
  }
  
  /*
   * PushChildNode() - Push child node into child node list
   */
  inline void PushChildNode(SyntaxNode *node_p) {
    child_list.push_back(node_p);
  }
  
  /*
   * ReverseChildList() - Reverse the child list in-place
   */
  inline void ReverseChildList() {
    std::reverse(child_list.begin(), child_list.end());
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
  
  // This is an auxiliary data structure that is used to store OpInfo
  // pointer corresponding to op_stack
  std::stack<const OpInfo *> op_info_stack;
  
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
   * PushOpNode() - Push an operator syntax node into the stack
   *
   * NOTE: We need to manually provide the op_info_p in the argument
   * since the caller of this function has to first get it from
   * the TokenInfo class, and we do not want to perform the same
   * action twice since querying TokenInfo is not a trivial task
   */
  inline void PushOpNode(SyntaxNode *node_p, const OpInfo *op_info_p) {
    op_stack.push(node_p);
    op_stack_last_modified = true;
    
    // Need to maintain a synchronized OpInfo stack
    op_info_stack.push(op_info_p);
    
    return;
  }
  
  /*
   * PopOpNode() - Pops an operator from the stack
   *
   * If the stack is empty just return nullptr and it should be
   * treated as an error
   */
  inline SyntaxNode *PopOpNode() {
    if(op_stack.size() == 0UL) {
      return nullptr;
    }
    
    SyntaxNode *node_p = op_stack.top();
    op_stack.pop();
    
    // Need to maintain synchronized opinfo stack
    op_info_stack.pop();
    
    return node_p;
  }
  
  /*
   * PushValueNode() - Push a syntax node into value stack
   *
   * This function will set op_stack_last_modified as false as a by-product
   */
  inline void PushValueNode(SyntaxNode *node_p) {
    value_stack.push(node_p);
    op_stack_last_modified = false;
    
    return;
  }
  
  /*
   * PopValueNode() - Returns a value SyntaxNode
   *
   * If the value stack is empty just return nullptr. This is almost an error
   */
  inline SyntaxNode *PopValueNode() {
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
  
  /*
   * GetOpStackSize() - Return the size of the operator stack
   */
  inline size_t GetOpStackSize() const {
    return op_stack.size();
  }
  
  /*
   * GetValueStackSize() - Return the size of the value stack
   */
  inline size_t GetValueStackSize() const {
    return value_stack.size();
  }
  
  /*
   * TopOpNode() - Returns the top on op_stack
   */
  inline SyntaxNode *TopOpNode() {
    assert(op_stack.size() > 0UL);
    
    return op_stack.top();
  }
  
  /*
   * TopOpInfo() - Access the OpInfo structure of the topmost operator
   */
  inline const OpInfo *TopOpInfo() {
    assert(op_stack.size() == op_info_stack.size());
    
    return op_info_stack.top();
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
    TokenType type = token_p->GetType();
    
    switch(type) {
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
        return type;
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
   *
   * The information of operator is also passed in as argument since
   * we need to query a hash table to get OpInfo which is a non-trivial task
   * so the number of queries should be reduced to a minimum
   */
  void ReduceOperator(ExpressionContext *context_p) {
    // We do not care about the actual type of the top operand here
    // just pop an operand and
    int operand_num = context_p->TopOpInfo()->operand_num;
    
    // We could not reduce T_PAREN
    assert(operand_num != -1);
    
    // The number of operands must be between [1, 3]
    // NOTE: For T_PAREN type the number of operand is -1
    // so it should not be reduced in this function
    assert(operand_num > 0);
    assert(operand_num <= 3);
    
    // No matter how many operands it requires, first check
    // the number of values in value stack
    if(operand_num > context_p->GetValueStackSize()) {
      ThrowMissingOperandError(operand_num, context_p->GetValueStackSize());
    }
    
    // Next we pop operand_num syntax nodes from the value stack
    // and reverse them in-place
    
    // We must have something to pop out in this function
    // if ther operator stack is empty it should be handled outside
    // of this function
    assert(context_p->GetOpStackSize() > 0UL);
    
    // This is the top of operator node
    SyntaxNode *top_op_node_p = context_p->PopOpNode();
    assert(top_op_node_p != nullptr);
    
    // Deliberately let it fall through to unroll the loop
    switch(operand_num) {
      case 3:
        top_op_node_p->PushChildNode(context_p->PopValueNode());
      case 2:
        top_op_node_p->PushChildNode(context_p->PopValueNode());
      case 1:
        top_op_node_p->PushChildNode(context_p->PopValueNode());
      default:
        assert(false);
    } // switch
    
    // Since we poped it from right to left, and the correct order is from
    // left to right for all operators
    top_op_node_p->ReverseChildList();
    
    // The last step is to push it back to the value stack since now
    // it is reduced from operator to a value
    context_p->PushValueNode(top_op_node_p);
    
    return;
  }
  
  /*
   * ReduceOnPrecedence() - Reduces operators until the top one is
   *                        of even lower precedence (or the same
   *                        precedence) of the current operator
   *
   * Note that we need to reduce to an operator of < or <= precedence
   * of the given one, depending on associativity. For left-to-right
   * assosiativity we should reduce to "<=" precedence, o.w. reduce
   * to a "<" operator is necessary
   *
   * NOTE: It does not push current operator into the stack, which must
   * be done by the caller
   */
  void ReduceOnPrecedence(ExpressionContext *context_p,
                          OpInfo *current_op_info_p) {
                            
    // If left-to-right order then loop until see a <= precedence
    // i.e. precedence numeric value >=
    // T_PATEN should also belong to this category since
    // we could not reduce parenthesis
    //
    // Since we resolve precedence first, and resolve associativity
    // only when the precedence is the same (in which case assosciativity
    // is the same), so we could test current operator's associativity
    if(current_op_info_p->associativity == EvalOrder::LEFT_TO_RIGHT) {
      
      while(context_p->GetOpStackSize() > 0) {
        // Get the top operator node's precedence
        const OpInfo *top_op_info_p = context_p->TopOpInfo();
        
        // If the top node has a lower or equal precedence than
        // the current testing node then return
        //
        // for T_PAREN this is always satisfied so we will not
        // reduce on parenthesis unless requested
        if(top_op_info_p->precedence >= current_op_info_p->precedence) {
          break;
        }
        
        // Reduce the operator, pop operands, push into child list, and
        // push the complex into the value list
        // NOTE that this will change IsPrefix() flag to false
        // since we have pushed into the value list
        ReduceOperator(context_p);
      }
    } else if(current_op_info_p->associativity == EvalOrder::RIGHT_TO_LEFT) {
      while(context_p->GetOpStackSize() > 0) {
        const OpInfo *top_op_info_p = context_p->TopOpInfo();

        if(top_op_info_p->precedence >= current_op_info_p->precedence) {
          break;
        }

        ReduceOperator(context_p);
      }
    } else {
      assert(false);
    }
    
    return;
  }
  
  
  void ReduceTillParenthesis(ExpressionContext *context_p) {
    
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
  
  ///////////////////////////////////////////////////////////////////
  // Error handling
  ///////////////////////////////////////////////////////////////////
  
  /*
   * ThrowMissingOperandError() - This is thrown when reducing operator but
   *                              there is missing operand
   */
  void ThrowMissingOperandError(int expected, int actual) {
    throw std::string{"Missing operand: expect "} + \
          std::to_string(expected) + \
          "; actual " + \
          std::to_string(actual);
  }
};
