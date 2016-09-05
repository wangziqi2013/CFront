
#pragma once

#include "common.h"

namespace wangziqi2013 {
namespace cfront {

/*
 * class SlabAllocator - Allocates elements but never frees them until explicit
 *                       call of this function
 *
 * This class is used for two purposes:
 *   1. For many small allocations, reduce call to malloc() to reduce
 *      memory overhead, fragmentation and time cost
 *   2. For shared pointer where ownership is not clear, act as a pool
 *      and removes the need to free node
 *
 * Note that this slab allocator is not thread-safe
 */
template <typename ElementType>
class SlabAllocator {
 private:
   
  // This is the stack where we hold chunks
  std::stack<char *> chunk_stack;
  
  // This is the index inside current (topmost) chunk
  int next_element_index;
  
  // Number of elements per chunk. This is configurable at compile
  // time to let the caller choose
  int element_per_chunk;
  
  /*
   * ThrowAllocatorOutOfMemoryError() - This is thrown when we are out of
   *                                    memory through malloc() call
   */
  void ThrowAllocatorOutOfMemoryError() const {
    throw std::string{"Slab allocator out of memory!"};
  }
  
 public:
   
  /*
   * Constructor - Initializes the stack and index structure
   */
  SlabAllocator(int p_element_per_chunk=64) :
    chunk_stack{},
    next_element_index{0};
    element_per_chunk{p_element_per_chunk} {
    // Allocate the first chunk of memory
    char *ptr = malloc(element_per_chunk * sizeof(ElementType));
    if(ptr == nullptr) {
      ThrowAllocatorOutOfMemoryError();
    }
  }
  
  ElementType *
  
  
};
  
}
}
