
#include <stdio.h>
#include <assert.h>
#include "stack.h"

void test_stack() {
  printf("=== Test Stack ===\n");
  stack_t *stack = stack_init();

  for(int i = 0;i < STACK_INIT_CAPACITY * 3 + 123;i++) {
    stack_push(stack, (void *)i);
  }

  for(int i = STACK_INIT_CAPACITY * 3 + 123 - 1;i >= 0;i--) {
    int ret = (int)stack_pop(stack);
    assert(ret == i);
  }

  assert(stack->size == 0);
  assert(stack->capacity > STACK_INIT_CAPACITY * 3);

  stack_free(stack);
  printf("Pass!\n");

  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_stack();
  return 0;
}