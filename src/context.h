
#pragma once

#include "common.h"
#include "token.h"
#include "scope.h"

namespace wangziqi2013 {
namespace cfront {

/*
 * class Context - The object for holding global values such as symbol tables
 *                 and type tables
 */
class Context {
 private:

  // Note that we could not use stack here since stack does not support
  // iteration, and during a search we have to iterate through the scope
  // to search a named type
  //
  // Use push_back() and pop_back() to access elements like a stack
  std::vector<ScopeNode> scope_stack;

 public:

  /*
   * Constructor
   *
   * The ownership of source file belongs to the context object
   */
  Context() :
    scope_stack{} {
    // We initialize the first level of stack using an empty scope
    // possibly with few built-in symbols (but not built-in types
    // because we )
  }

  /*
   * EnterScope() - Pushes a new ScopeNode object into the stack
   *                and return the pushed object
   */
  ScopeNode &EnterScope() {
    // Construct an empty scope node and pusu it back to the vector
    scope_stack.emplace_back();
    
    return scope_stack.back();
  }
  
  /*
   * LeaveScope() - Leaves the scope by popping the node out from the stack
   *
   * If the scope stack is already empty then the assertion would fail
   */
  void LeaveScope() {
    assert(scope_stack.size() > 0);
    
    scope_stack.pop_back();
    
    return;
  }
  
  /*
   * GetTypeNode() - Search on the stack for a named type
   *
   * This function searches the stack from the top top the bottom, and if
   * the name exists inside any level that are searched first then it returns
   * the associated type object
   */
  SyntaxNode *GetTypeNode(const std::string &type_name) {
    // Iterate through the vector from high index to low index
    // i.e. from most recent name space to less recent ones
    for(auto it = scope_stack.rbegin(); it != scope_stack.rend();it++) {
      SyntaxNode *type_node_p = it->GetTypeNode(type_name);
      
      // If the type exists in the scope being searched just return it
      // Otherwise need to continue to the next scope
      if(type_node_p != nullptr) {
        return type_node_p;
      }
    }
    
    // If at last we did not find such name then the type does
    // not exist and return nullptr
    return nullptr;
  }
};

} // namespace wangziqi2013
} // namespace cfront
