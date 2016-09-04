
#pragma once

#include "common.h"

namespace wangziqi2013 {
namespace cfront {

class SyntaxNode;

/*
 * class ScopeNode - This is a structure that contains information
 *                   about a scope.
 *
 * ScopeNodes are put into a stack as translation units are entered
 * and exited
 */
class ScopeNode {
 private:
  // The set of types we have currently seen
  // Types are represented using SyntaxNode structure which means it
  // could be organized as a tree
  //
  // For the topmost level in this structure we should put 8 basic types:
  // char, short, int, long
  // unsigned char, unsigned short, unsigned int, unsigned long
  std::unordered_map<std::string, SyntaxNode *> type_map;
  
  // TODO: Change the mapped type to something more meaningful
  // This should be
  std::unordered_map<std::string, int> ident_map;
 public:

  /*
   * Constructor - This is necessary for emplacing it back in a stack
   */
  ScopeNode() {}
  
  /*
   * These are deleted to avoid any undesirable effects
   */
  ScopeNode(const ScopeNode &) = delete;
  ScopeNode(ScopeNode &&) = delete;
  ScopeNode &operator=(const ScopeNode &) = delete;
  ScopeNode &operator=(ScopeNode &&) = delete;
  
  /*
   * GetTypeNode() - Return the type node from type map
   *
   * If the type has not yet been defiend just return nullptr
   */
  SyntaxNode *GetTypeNode(const std::string &type_name) {
    auto it = type_map.find(type_name);
    
    // If the type does not exist in the map just return nullptr
    if(it == type_map.end()) {
      return nullptr;
    }
    
    return it->second;
  }
  
  /*
   * GetTypeMap() - Return the type map object reference
   *
   * The return value is a non-const reference which means that we could
   * actually modify it
   */
  std::unordered_map<std::string, SyntaxNode *> &
  GetTypeMap() {
    return type_map;
  }
};

} // namespace wangziqi2013
} // namespace cfront
