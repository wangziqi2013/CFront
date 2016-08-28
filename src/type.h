
#pragma once

#include "common.h"
#include <vector>

namespace wangziqi2013 {
namespace cfront {

enum class TypeNodeType {
  // Primitive Integer types
  // There is a bit width field indicating how many bits is used
  // to represent this type
  //
  // It covers the usage of bit field
  INTEGER_T,
  
  // Array Type
  ARRAY_T = 100,
  
  // Struct Type
  STRUCT_T = 200,
  
  // Union Type
  UNION_T = 300,
  
  // Enum Type
  ENUM_T = 400,
};

/*
 * class TypeNode - Stores either promitive type information or synthesized
 *                  type information from other types
 */
class TypeNode {
 private:
  // This is the type of nodes - could either be primitive types or
  // compound types
  TypeNodeType type;
  
  
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
