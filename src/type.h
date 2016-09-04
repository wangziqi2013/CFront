
#pragma once

#include "common.h"
#include "token.h"

namespace wangziqi2013 {
namespace cfront {

/*
 * class TypeNode - Stores either promitive type information or synthesized
 *                  type information from other types
 */
class TypeNode {
 private:
  // This is the type of nodes - could either be primitive types or
  // compound types
  TokenType type;
  
  
  union {
    // The length of bits if it is an integer type
    int bit_width;
    
    // Holds children nodes if it is a composite type
  };
  
 public:

  /*
   * GetType() - Returns the type of the node
   */
  TypeNodeType GetType() const {
    return type;
  }
  
  /*
   * GetBitWidth() - Returns the number of bits inside the integer type
   *
   * We check whether the node is of INTEGER_T type. If not then assertion
   * fails
   */
  int GetBitWidth() const {
    assert(type == TypeNodeType::INTEGER_T);
    
    return bit_width;
  }
};

} // namespace cfront
} // namespace wangziqi2013
