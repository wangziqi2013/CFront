
#pragma once

#include "common.h"

#include "lex.h"
#include "token.h"
#include "context.h"
#include "scope.h"

namespace wangziqi2013 {
namespace cfront {

/////////////////////////////////////////////////////////////////////
// class SyntaxNode
/////////////////////////////////////////////////////////////////////

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
   * TraversePrint() - Traverse and print all nodes in prefix order
   */
  void TraversePrint(int level = 0) const {
    std::string s = token_p->ToString();
    
    // Print i space for identation
    for(int i = 0;i < level;i++) {
      printf("    ");
    }
    
    printf("%s\n", s.c_str());
    
    int counter = 0;
    for(SyntaxNode *node_p : child_list) {
      for(int i = 0;i < level;i++) {
        printf("    ");
      }
      
      printf("(Child %d)\n", counter++);
      
      node_p->TraversePrint(level + 1);
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

/////////////////////////////////////////////////////////////////////
// class SyntaxNode ends
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// class ExpressionContext
/////////////////////////////////////////////////////////////////////

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
  //
  // One exception is for unary postfix operator, since they are
  // associated left-to-right, for a ++ ++, after seeing the first ++
  // and when we see the second ++, there will be no reduction, so the
  // last stack modified is op stack, but semantically is_prefix should
  // be false. We should treat this differently
  bool is_prefix;
  
  // This counts the numebr of outstanding '(' that have not yet been matched
  // by a corresponding ')'.
  //
  // If this counter is 0 then every ')' we have seen in the input stream is
  // a signal of stopping parsing. Otherwise every ')' is a sign to
  // do reduction
  int parenthesis_counter;
  
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
    is_prefix{true},
    parenthesis_counter{0}
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

    // For a ++ ++ we should keep is_prefix as false
    if(op_info_p->is_postfix_unary == false) {
      is_prefix = true;
    }
    
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
   * This function will set is_prefix as false as a by-product
   */
  inline void PushValueNode(SyntaxNode *node_p) {
    value_stack.push(node_p);
    
    // This is always true, no special treatment
    is_prefix = false;
    
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
    value_stack.pop();
    
    return node_p;
  }
  
  /*
   * IsPrefix() - Returns whether the op stack is the last stack we have
   *              modified
   *
   * This is equivalent to whether the coming operator is prefix operator
   * or not since if op stack is the last stack modified then we have seen
   * an op and is expceting a value or a prefix op
   *
   * The only exception is for unary postfix operator, where "a" "++" "++"
   * will be recognized as ++a++ since the second ++ is mistakenly recognized
   * as another ++ before an operand
   */
  bool IsPrefix() const {
    return is_prefix;
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
   * TopValueNode() - Returns the top node on the value stack
   *
   * This function requires that the value stack has at least 1 element
   */
  inline SyntaxNode *TopValueNode() {
    assert(value_stack.size() > 0UL);
    
    return value_stack.top();
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
  
  /*
   * EnterParenthesis() - Increases the parenthesis counter by 1
   *
   * This is called if we see a '(' that represents a left parenthesis (i.e.
   * in its prefixed form)
   */
  inline void EnterParenthesis() {
    assert(parenthesis_counter >= 0);
    
    parenthesis_counter++;
    
    return;
  }
  
  /*
   * LeaveParenthesis() - Decreases the parenthesis counter by 1
   *
   * This is called if we see a ')' that represents a right parenthesis
   */
  inline void LeaveParenthesis() {
    assert(parenthesis_counter >= 1);
    
    parenthesis_counter--;
    
    return;
  }
  
  /*
   * IsInParenthesis() - Returns whether there is any outstanding '(' to
   *                     be matched
   */
  inline bool IsInParenthesis() {
    assert(parenthesis_counter >= 0);
    
    // If it is >0 then we know there is one '(' not matched
    return (parenthesis_counter > 0);
  }
  
  // The following functions deals with reduce and stack operation
  
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
  void ReduceOperator() {
    // We do not care about the actual type of the top operand here
    // just pop an operand and
    int operand_num = TopOpInfo()->operand_num;

    // We could not reduce T_PAREN
    assert(operand_num != -1);

    // The number of operands must be between [1, 3]
    // NOTE: For T_PAREN type the number of operand is -1
    // so it should not be reduced in this function
    assert(operand_num > 0);
    assert(operand_num <= 3);

    // No matter how many operands it requires, first check
    // the number of values in value stack
    if(operand_num > GetValueStackSize()) {
      ThrowMissingOperandError(operand_num, GetValueStackSize());
    }

    // Next we pop operand_num syntax nodes from the value stack
    // and reverse them in-place

    // We must have something to pop out in this function
    // if ther operator stack is empty it should be handled outside
    // of this function
    assert(GetOpStackSize() > 0UL);

    // This is the top of operator node
    SyntaxNode *top_op_node_p = PopOpNode();
    assert(top_op_node_p != nullptr);

    // Deliberately let it fall through to unroll the loop
    switch(operand_num) {
      case 3:
        top_op_node_p->PushChildNode(PopValueNode());
      case 2:
        top_op_node_p->PushChildNode(PopValueNode());
      case 1:
        top_op_node_p->PushChildNode(PopValueNode());
        break;
      default:
        dbg_printf("Operand num = %d; error!\n", operand_num);

        assert(false);
    } // switch

    // Since we poped it from right to left, and the correct order is from
    // left to right for all operators
    top_op_node_p->ReverseChildList();

    // The last step is to push it back to the value stack since now
    // it is reduced from operator to a value
    PushValueNode(top_op_node_p);

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
  void ReduceOnPrecedence(const OpInfo *current_op_info_p) {
    dbg_printf("Reduce on operator of precedence %d\n",
               current_op_info_p->precedence);

    // If left-to-right order then loop until see a < precedence
    // i.e. precedence numeric value >
    //
    // Since we resolve precedence first, and resolve associativity
    // only when the precedence is the same (in which case assosciativity
    // is the same), so we could test current operator's associativity
    if(current_op_info_p->associativity == EvalOrder::LEFT_TO_RIGHT) {

      while(GetOpStackSize() > 0) {
        // Get the top operator node's precedence
        const OpInfo *top_op_info_p = TopOpInfo();

        // If the top node has a lower or equal precedence than
        // the current testing node then return
        if(top_op_info_p->precedence > current_op_info_p->precedence) {
          break;
        }

        // Reduce the operator, pop operands, push into child list, and
        // push the complex into the value list
        // NOTE that this will change IsPrefix() flag to false
        // since we have pushed into the value list
        ReduceOperator();
      }
    } else if(current_op_info_p->associativity == EvalOrder::RIGHT_TO_LEFT) {
      while(GetOpStackSize() > 0) {
        const OpInfo *top_op_info_p = TopOpInfo();

        if(top_op_info_p->precedence >= current_op_info_p->precedence) {
          break;
        }

        ReduceOperator();
      }
    } else {
      assert(false);
    }

    return;
  }
  
  /*
   * ReduceTillParenthesis() - Reduce top level until we see a T_PAREN
   *
   * If there is no T_PAREN in the stack then raise an error, since ")"
   * and "(" are not balanced
   *
   * NOTE: This function keeps reducing when we have seen a T_LPAREN
   * on the op stack, this is to make sure there is at least 1 syntax
   * element inside the parenthesis
   */
  void ReduceTillParenthesis() {
    dbg_printf("Reduce till parentheis\n");

    // If we get out of this while loop then there is an error
    while(GetOpStackSize() > 0) {
      TokenType top_op_type = TopOpNode()->GetType();

      // No matter what it is as long as we have not exited
      // we always do a reduce, including for T_PAREN
      ReduceOperator();

      // We use the saved top op type since the top op has changed
      if(top_op_type == TokenType::T_PAREN) {
        return;
      }
    } // while stack is not empty

    // If we have emptied the stack and still did not see "("
    // then throw error since there is no matching for ")"
    ThrowMissingLeftParenthesisError();

    assert(false);
    return;
  }
  
  /*
   * ReduceTillEmpty() - Reduce all operators on the stack until
   *                     the op stack is empty
   *
   * This function is called when we know we have reached the end of the
   * expression
   */
  void ReduceTillEmpty() {
    // Loop until the stack is finally empy
    while(GetOpStackSize() > 0) {
      ReduceOperator();
    } // while stack is not empty

    return;
  }
  
  ///////////////////////////////////////////////////////////////////
  // Error handling
  ///////////////////////////////////////////////////////////////////
  
  /*
   * ThrowMissingLeftParenthesisError() - This is thrown when we reduce to
   *                                      right parenthesis but could not find
   *                                      it in the operator stack
   */
  void ThrowMissingLeftParenthesisError() const {
    throw std::string{"Unmatched '(' and ')'; missing '('"};
  }
  
  /*
   * ThrowMissingOperandError() - This is thrown when reducing operator but
   *                              there is missing operand
   */
  void ThrowMissingOperandError(int expected, int actual) const {
    throw std::string{"Missing operand: expect "} + \
          std::to_string(expected) + \
          "; actual " + \
          std::to_string(actual);
  }
};

/////////////////////////////////////////////////////////////////////
// class ExpressionContext ends
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// class TypeParser
/////////////////////////////////////////////////////////////////////

/*
 * class TypeParser - Parses a type expression and represent it using
 *                    SyntaxNode * and its trees
 */
class TypeParser {
 private:
  // This is the tokenizer
  // Ownership of the tokenizer does not belong to this class
  // so the source file object should be destroyed separately
  SourceFile *source_p;
  
  // This is the context object that holds global information such as
  // symbol table and type table and stacks for both of them
  Context *context_p;

 public:

  /*
   * Constructor
   */
  TypeParser(SourceFile *p_source_p,
             Context *p_context_p) :
    source_p{p_source_p},
    context_p{p_context_p}
  {}

  /*
   * Destructor
   */
  ~TypeParser() {}

  /*
   * Deleted function
   */
  TypeParser(const TypeParser &) = delete;
  TypeParser(TypeParser &&) = delete;
  TypeParser &operator=(const TypeParser &) = delete;
  TypeParser &operator=(TypeParser &&) = delete;

  /*
   * TryParseBaseType() - Try to parse a base type from the input
   *                      token stream
   *
   * This function is allowed to fail sometimes (majorly when we have seen a
   * left parenthesis and are not sure whether it is a real parenthesis or a
   * type cast). If it could not parse a base type from the stream just
   * return nullptr and the token fetched from the stream will be pushed back
   *
   * If a type could be found then it returns a SyntaxNode * wrapping the
   * token object extracted from the stream
   *
   * The logic of this function is described below:
   *   1. If the first token is struct, then we know the following token
   *      is going to be an ident that is a registered type. All other tokens
   *      will be treated as a tpye not found error. Note that in this case
   *      we do not return nullptr but directly go to error since struct
   *      suggests a type.
   *   2. If the first token is an ident then we try to find it in the context
   *      as a registered type. If type not found then push back and return
   *      nullptr
   *   3. If it is signed, unsigned, or char, short, int, long then we continue
   *      to retrieve the entire type
   */
  SyntaxNode *TryParseBaseType() {
    Token *token_p = source_p->GetNextToken();
    
    //if()
  }

  /*
   * ParseTypeExpression() - Parses the expression part of a type
   *
   * A type has two parts - base type part and expression part.
   * This function is responsible for parsing the expression part
   * of a type declaration
   */
  SyntaxNode *ParseTypeExpression() {
    ExpressionContext context{};
    
    while(1) {
      Token *token_p = source_p->GetNextToken();
      TokenType type = token_p->GetType();
      
      switch(type) {
        // '*' is always dereference
        case TokenType::T_STAR:
          type = TokenType::T_DEREF;
          break;
        // '[' is always array sub
        case TokenType::T_LSPAREN:
          type = TokenType::T_ARRAYSUB;
          break;
        // '(' could be function call or left parenthesis
        // we need to distinguish between these two cases
        case TokenType::T_LPAREN:
          
          break;

        // By default the type is not changed
        default:
          break;
      } // switch type
      
      // Set the type of the token to be the mapped type
      token_p->SetType(type);
    } // while(1)
    
    assert(false);
    return nullptr;
  }

  /*
   * ParseType() - Parses a type expression
   */
  SyntaxNode *ParseType() {
    assert(false);
    return nullptr;
  }
};

/////////////////////////////////////////////////////////////////////
// class TypeParser ends
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// class ExpressionParser
/////////////////////////////////////////////////////////////////////

/*
 * class ExpressionParser - Analyzes expressions and builds syntax tree
 *
 * We use stack based shift-reduce parsing tachnique here
 */
class ExpressionParser {
 private:
  // This holds raw data and also acts as a tokenizer
  // The syntax analyzer does not have ownership of this pointer
  // and the source file should be destroyed separately
  SourceFile *source_p;
  
  // We need this for the type table when dealing with type casting
  Context *context_p;

 public:
   
  /*
   * Constructor
   */
  ExpressionParser(SourceFile *p_source_p, Context *p_context_p) :
    source_p{p_source_p},
    context_p{p_context_p}
  {}
  
  /*
   * Destructor
   */
  ~ExpressionParser() {}
  
  /*
   * Deleted function
   */
  ExpressionParser(const ExpressionParser &) = delete;
  ExpressionParser(ExpressionParser &&) = delete;
  ExpressionParser &operator=(const ExpressionParser &) = delete;
  ExpressionParser &operator=(ExpressionParser &&) = delete;
  
  /*
   * GetExpressionNodeType() - Return the real type of a syntax node
   *
   * Since many operators are used in an implicitly overloaded manner, e.g.
   * "*" is either used as multiplication and pointer dereference. We need to
   * determine its type based on whether it is used in prefix or postfix
   * form.
   *
   * NOTE: If the is_prefix flag is true then we are in prefix form
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
      
      dbg_printf("Type after conversion: %d\n", (int)type);
      
      // And reset type to reflect prefix/postfix status
      //
      // For those not part of the expression or do not have prefix/postfix
      // form this does not change but still we set it
      token_p->SetType(type);
      
      // If it is not found then check whether it is ). If not then
      // we know we have reached the end
      //
      // This includes ',' and ']' which will cause this function to return
      // ',' is useful in parameter list parsing
      // and ']' is useful in array index parsing
      const OpInfo *op_info_p = TokenInfo::GetOpInfo(type);
      
      // Either it is not part of the expression, or it is
      // a data terminal
      //
      // ')' ']' ',' will lead into this block
      if(op_info_p == nullptr) {
        
        // If it is terminal types that carries data then
        // push them into value stack
        if(type == TokenType::T_STRING_CONST ||
           type == TokenType::T_INT_CONST ||
           type == TokenType::T_IDENT ||
           type == TokenType::T_CHAR_CONST) {
          dbg_printf("See data terminal node. Push and start new loop\n");
          
          // Construct a syntax node and initialize it with the token
          // object
          // The token object will not be destroyed until the syntax node
          // is destructed
          context.PushValueNode(new SyntaxNode{token_p});
          
          // Start with next token
          continue;
        }

        // If we do not terminate on ')', i.e. this is not a function call
        // argument extraction recursive call
        // then just reduce until parenthesis and pop the parenthesis node
        // (i.e. parenthesis node only has one child, so it is meangingless to
        // let it appear in expression tree)
        //
        // Note that here we also need to check whether the parenthesis
        // is the one we are looking for (i.e. whether we are currently
        // inside some parenthesis, since if there is a function call:
        //   "f(a, b, (a + b))", then when parsing (a + b) it will not
        // terminate on the second ')' because it tries to reduce on that
        // token, which is undesired. To make the parse be aware of a possible
        // T_RPAREN that might not belong to the current expression the
        // IsInParenthesis() call is important
        if((type == TokenType::T_RPAREN) && \
           (context.IsInParenthesis() == true)) {
          // Leave current parenthesis
          context.LeaveParenthesis();
          
          context.ReduceTillParenthesis();
          
          SyntaxNode *paren_node_p = context.PopValueNode();
          assert(paren_node_p->GetType() == TokenType::T_PAREN);
          
          // Make sure there is only 1 element in the parenthesis syntax node
          assert(paren_node_p->GetChildList().size() == 1UL);
          
          // This is its direct child node which we will push into the stack
          SyntaxNode *child_node_p = paren_node_p->GetChildList()[0];
          
          // Make sure when we destroy it the child not will not be destroyed
          paren_node_p->GetChildList().clear();
          delete paren_node_p;
          
          context.PushValueNode(child_node_p);;
          
          continue;
        }

        // Push it back
        // If it is ',' or ')' in param list then caller should extract
        // and verify them
        //
        // If it is ']' caller should also verify them
        source_p->PushBackToken(token_p);

        dbg_printf("Push back token of type %d\n", (int)type);

        // Reduce all operators and see whether there is single value in
        // value stack
        // If not then we have error
        context.ReduceTillEmpty();
        if(context.GetValueStackSize() != 1) {
          ThrowUnexpectedValueError(context.GetValueStackSize());
        }
        
        // Pop the only value object and return it
        return context.PopValueNode();
      } // if (not a regular operator, i.e. RPAREN, ',' or ']')
      
      //
      // After this point we know the token must be some operator and
      // the expression has not ended yet grammatically
      //
      
      if(type == TokenType::T_ARRAYSUB) {
        // Since [] is among the highest precedence operators
        // it could only cause reduce on the same class (i.e. unary postfix)
        // e.g. a--[2] will cause reduction of "--" when seeing '['
        context.ReduceOnPrecedence(op_info_p);

        // Push the [] into the op stack
        // Though this is not strictly necessary we do that to simplify
        // later processing because we could call reduce
        context.PushOpNode(new SyntaxNode{token_p}, op_info_p);

        // Parse the expression inside the [] until it returns
        // it returns on ']' and ','
        SyntaxNode *sub_node_p = ParseExpression();

        // The other token must be ']'
        Token *end_token_p = source_p->GetNextToken();
        TokenType end_token_type = end_token_p->GetType();
        
        // No matter it is ']' or not we do not need it
        delete end_token_p;
        
        if(end_token_type != TokenType::T_RSPAREN) {
          // It will delete all its child and below recursively
          delete sub_node_p;

          ThrowMissingRightSolidParenthesisError(end_token_p->GetType());
        }
        
        // We need to satisfy the operand number requirement
        context.PushValueNode(sub_node_p);
        
        // This will reduce ARRAYSUB and push into value stack
        // so it unsets is_prefix flag which is good since any operand
        // after arraysub must be postfix
        context.ReduceOperator();
        
        continue;
      } else if(type == TokenType::T_FUNCCALL) {
        // Since T_FUNCCALL is also among the highest precedence operators
        // it could only reduce on the same class (i.e. unary postfix)
        context.ReduceOnPrecedence(op_info_p);

        // Push the T_FUNCCALL into the op stack
        // Though this is not strictly necessary we do that to simplify
        // later processing because we could call reduce
        context.PushOpNode(new SyntaxNode{token_p}, op_info_p);

        // This subroutine basically try to parse expressions from
        // function argument list, until it sees ) and then it returns
        //
        // It pushes a single SyntaxNode of type FUNCARG into the value stack
        // such that no matter how many arguments there are for a function
        // call, we should always have exactly 2 arguments for T_FUNCCALL
        SyntaxNode *arg_node_p = ParseFunctionArgumentList();

        // We need to satisfy the operand number requirement
        context.PushValueNode(arg_node_p);

        context.ReduceOperator();
        
        continue;
      }
      
      // Record every parenthsis we enter
      if(type == TokenType::T_PAREN) {
        context.EnterParenthesis();
        
        // For parenthesis we do not reduce on precedence since its
        // precedence is very very low so it will cause
        // reduction on any other operator which is very bad
        context.PushOpNode(new SyntaxNode{token_p}, op_info_p);
        
        continue;
      }
      
      // We know op_info_p is not nullptr
      context.ReduceOnPrecedence(op_info_p);
      
      // Now we could push into the op stack and continue
      context.PushOpNode(new SyntaxNode{token_p}, op_info_p);
    } // while(1)
    
    assert(false);
    return nullptr;
  }
  
  /*
   * ParseFunctionArgumentList() - Parse function argument list into a
   *                               single syntax node and return it
   *
   * This function recursively calls ParseExpression() on each argument,
   * and configure ParseExpression() that it returns on ',' and especially
   * on ')'
   */
  SyntaxNode *ParseFunctionArgumentList() {
    dbg_printf("Parsing function argument...\n");
    
    // Create a new syntax node for holding parameter values
    //
    // It also needs to carry a token node of type T_FUNCARG
    SyntaxNode *arg_node_p = new SyntaxNode{new Token{TokenType::T_FUNCARG}};
    
    Token *token_p = source_p->GetNextToken();

    // If it is ')' then we know it is time to exit
    // This is a special case then we do not allow expression to be
    // an empty string, so "func()" will see ')' but no expression
    // when being parsed
    if(token_p->GetType() == TokenType::T_RPAREN) {
      // It is useless
      delete token_p;

      return arg_node_p;
    } else {
      source_p->PushBackToken(token_p);
    }
    
    // After this point we know there is at least 1 expression in the
    // argument list
    
    while(1) {
      // Since the parser try to balance parenthesis inside
      // an expression, when seeing ')', if parenthesis has already
      // been balanced then the parser knows it is outside the current
      // expression and will just return
      SyntaxNode *node_p = ParseExpression();
      arg_node_p->PushChildNode(node_p);

      token_p = source_p->GetNextToken();
      
      // If the next token is ')' then function parsing is done
      // If the next token is ',' then we have more arguments to process
      // In all other cases throw an error - unexpected symbol when parsing
      // function argument list
      if(token_p->GetType() == TokenType::T_RPAREN) {
        // It is useless
        delete token_p;

        return arg_node_p;
      } else if(token_p->GetType() == TokenType::T_COMMA) {
        // Delete it
        delete token_p;
        
        // Comma is a hint to continue collection more expressions
      } else {
        ThrowUnexpectedFuncArgSep(token_p->GetType());
      }
    } // while(1)
    
    // Control flow will never reach here
    assert(false);
    return nullptr;
  }
  
  ///////////////////////////////////////////////////////////////////
  // Error handling
  ///////////////////////////////////////////////////////////////////
  
  /*
   * ThrowUnexpectedFuncArgSep() - This is called when we see something other
   *                               than ',' and ')' just after an argument
   *                               expression of a function
   */
  void ThrowUnexpectedFuncArgSep(TokenType type) {
    throw std::string{"Unexpected separator after function argument: "} + \
          TokenInfo::GetTokenName(type);
  }
  
  /*
   * ThrowMissingRightSolidParenthesisError() - As name suggests
   *
   * NOTE: This function is different from the missing '(' in a sense
   * that we parse '[' and ']' recursively
   */
  void ThrowMissingRightSolidParenthesisError(TokenType type) const {
    throw std::string{"Unmatched '[' and ']': missing ']'; saw type "} + \
          std::to_string(static_cast<int>(type));
  }
  
  /*
   * ThrowUnexpectedValueError() - When we finishes an expression and reduces
   *                               until the operator stack becomes empty
   *                               if the value stack does not have exactly 1 
   *                               element then raise this error
   */
  void ThrowUnexpectedValueError(size_t num) const {
    throw std::string{"Unexpected value object"
                      " after reducing expression tree ("} + \
          std::to_string(num) + \
          " left)";
  }
};

/////////////////////////////////////////////////////////////////////
// class ExpressionParser ends
/////////////////////////////////////////////////////////////////////

}
}
