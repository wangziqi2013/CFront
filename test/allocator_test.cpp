
#include "test_suite.h"
#include <vector>

/*
 * class A - Testing class with a counter
 */
class A {
 public:
  // Number of objeces that are still alive
  static int counter;
  
  int data;
  
  /*
   * Constructor - Initialize data member to verify correctness of
   *               allocation
   */
  A(int p_data) :
    data{p_data} {
    counter++;
  }
  
  /*
   * Destructor
   */
  ~A() {
    counter--;
  }
};

int A::counter = 0;

/*
 * AllocTest1() - Tests initialization, allocate and destruction
 */
void AllocTest1(int element_per_chunk) {
  dbg_printf("Allocator test: element_per_cuont = %d\n",
             element_per_chunk);
  
  // Use default element per chunk = 64
  SlabAllocator<A> *alloc_p = new SlabAllocator<A>{element_per_chunk};
  std::vector<A *> a_list{};
  
  static const int num = 65555;
  
  // Intentionally make it an odd number
  for(int i = 0;i < num;i++) {
    a_list.push_back(alloc_p->Get(i));
  }
  
  assert(A::counter == num);
  
  // Verify whether data gets overwritten
  for(int i = 0;i < num;i++) {
    assert(a_list[i]->data == i);
  }
  
  // Call destructor to free all chunks and to
  // call destructors
  delete alloc_p;
  
  assert(A::counter == 0);
  
  return;
}

int main() {
  // Test with 64 elements per chunk
  AllocTest1(64);
  
  // Then test with 1 element per chunk
  AllocTest1(1);
  
  return 0;
}
