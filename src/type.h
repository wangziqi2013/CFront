
#pragma once

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
  TypeNodeType type;
};

} // namespace cfront
} // namespace wangziqi2013
