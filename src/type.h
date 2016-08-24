
#pragma once

#include <vector>

namespace wangziqi2013 {
namespace cfront {

enum class TypeNodeType {
  // Primitive Integer types
  INT8_T,
  UINT8_T,
  INT16_T,
  UINT16_T,
  INT32_T,
  UINT32_T,
  INT64_T,
  UINT64_T,
  
  // In the future probably we want to add bit field or even
  // arbitraty length type here
  //
  // But for now let's ignore these dark corners
  
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
