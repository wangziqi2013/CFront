
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
  std::stack<ScopeNode> scope_stack;

 public:

  /*
   * Constructor
   *
   * The ownership of source file belongs to the context object
   */
  Context() :
    scope_stack{}
  {}

  /*
   * EnterScope() - Pushes a new ScopeNode object into the stack
   *                and return the pushed object
   */
  ScopeNode &EnterScope() {
    scope_stack.emplace();
    
    return scope_stack.top();
  }
  
  /*
   * LeaveScope() - Leaves the scope by popping the node out from the stack
   *
   * If the scope stack is already empty then the assertion would fail
   */
  void LeaveScope() {
    assert(scope_stack.size() > 0);
    
    scope_stack.pop();
    
    return;
  }
};

} // namespace wangziqi2013
} // namespace cfront
