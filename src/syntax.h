
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
   * ParseExpression() - Parse expression using a stack and return the top node
   */
  SyntaxNode *ParseExpression() {
    std::stack<SyntaxNode *> op_stack{};
    std::stack<SyntaxNode *> node_stack{};
    
    // If the last modified stack is op stack then this is set to true
    bool op_stack_last_modified = true;
    
    // The following two functions are used to synvhronize two stacks
    // and the boolean variable
    
    // This lambda function is used to push a token into the op stack
    // Note that it uses a token node rather than syntax node
    auto push_op_stack = [&](Token *token_p) {
      op_stack.push(new SyntaxNode{token_p});
      op_stack_last_modified = true;
      
      return;
    }
    
    // This function pushes a syntax node into the node stack
    // Note that it uses a syntax node as argument
    auto push_node_stack = [&](SyntaxNode *node_p) {
      node_stack.push(node_p);
      op_stack_last_modified = false;

      return;
    }
    
    
    
    // Loops until we have seen a non-expression element
    // we need to push that token back before returning
    while(1) {
      Token *token_p = source_p->GetNextToken();
      TokenType type = token_p->GetType();

      switch(type) {
        // For the following operators they should be treated as unary operator
        case TokenType::T_STAR:
          // Prefix * is dereference
          token_p->SetType(TokenType::T_DEREF);
          
          // These two must appear together
          op_stack.push(new SyntaxNode{token_p});
          op_stack_last_modified = true;
        case TokenType::T_AMPERSAND:
        case TokenType::T_INC:
        case TokenType::T_DEC:
        //case TokenType::T_
      }
    }
  }
};
