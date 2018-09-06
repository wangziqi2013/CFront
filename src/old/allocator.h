
#pragma once

#include "common.h"

namespace wangziqi2013 {
namespace cfront {

/*
 * class SlabAllocator - Allocates elements but never frees them until explicit
 *                       call of free function
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
   * AllocateChunk() - Allocate a chunk and push it to the top of the stack
   *
   * This function also resets next_element_index to be 0 in order to use
   * the topmost chunk
   */
  void AllocateChunk() {
    // Allocate the first chunk of memory
    char *ptr = \
      reinterpret_cast<char *>(malloc(element_per_chunk * sizeof(ElementType)));

    if(ptr == nullptr) {
      ThrowAllocatorOutOfMemoryError();
    }

    chunk_stack.push(ptr);
    
    next_element_index = 0;
    
    return;
  }
  
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
    next_element_index{0},
    element_per_chunk{p_element_per_chunk} {
    
    // As part initialization allocate the first chunk on the internal stack
    AllocateChunk();
    
    return;
  }
  
  /*
   * Destructor - Frees all memory chunks in the slab allocator
   */
  ~SlabAllocator() {
    // Since we have next_element_index elements in the topmost chunk
    // just destruct the first next_element_index elements
    CallDestructorForEachElement(next_element_index);
    
    // Delete entire chunk of memory which is char * type
    free(chunk_stack.top());
    
    // Pop one chunk. We know there is at least one chunk on the stack
    chunk_stack.pop();
    
    // Delete all chunks until the stack is empty
    while(chunk_stack.size() > 0) {
      // Here since all other chunks are full, just delete chunks
      // using max element count as element to delete
      CallDestructorForEachElement(element_per_chunk);
      
      free(chunk_stack.top());
      
      chunk_stack.pop();
    }
    
    dbg_printf("Allocator finished cleanup\n");
    
    return;
  }
  
  /*
   * Get() - Returns an element type pointer allocated from the current chunk
   *
   * Note that the use of template class here is to let compiler construct
   * different Get() instances to forward constructor arguments to the
   * placement new which might take arguments
   *
   * These template arguments do not have to be explicitly specified since
   * the compiler could deduct them during compilation
   */
  template <typename ...Args>
  ElementType *Get(Args&&... args) {
    // If we have used up all slots in the current chunk
    // just allocate a new one and reset next element index to 0
    if(next_element_index == element_per_chunk) {
      AllocateChunk();
      
      assert(next_element_index == 0);
    }
    
    // This is the byte offset of the element being
    // allocated
    int byte_offset = sizeof(ElementType) * next_element_index;
    
    // Add the top most chunk address with the byte offset to yield element
    // address
    ElementType *element_ptr = \
      reinterpret_cast<ElementType *>(chunk_stack.top() + byte_offset);
      
    // Do not forget this!!!
    next_element_index++;
      
    // The last step is to call placement operator new to initialize the
    // object
    return new (element_ptr) ElementType{args...};
  }
  
  /*
   * CallDestructorForEachElement() - Calls destructor for the topmost chunk
   *
   * This function takes an extra argument as the element count on the top
   * most chunk, since it might or might not be the capacity of each chunk
   * caller needs to pass it in as an argument
   */
  void CallDestructorForEachElement(int element_count) {
    for(int i = 0;i < element_count;i++) {
      // Compute element pointer
      ElementType *ptr = \
        reinterpret_cast<ElementType *>(chunk_stack.top() +
                                        sizeof(ElementType) * i);

      // Call destructor manually
      ptr->~ElementType();
    }
    
    return;
  }
};
  
}
}
