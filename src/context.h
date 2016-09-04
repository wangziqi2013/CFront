
#pragma once

#include "common.h"
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
  void EnterScope() {
    
  }
  
  void LeaveScope() {

  }
};

} // namespace wangziqi2013
} // namespace cfront
