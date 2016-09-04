
#pragma once

#include <unordered_map>

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
  std::unordered_map<std::string, SyntaxNode *> type_set;
  
  // TODO: Change the mapped type to something more meaningful
  // This should be
  std::unordered_map<std::string, int> ident_set;
};
