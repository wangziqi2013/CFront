
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"

void test_stack() {
  printf("=== Test Stack ===\n");
  stack_t *stack = stack_init();

  for(long i = 0;i < STACK_INIT_CAPACITY * 3 + 123;i++) {
    stack_push(stack, (void *)i);
  }

  for(long i = STACK_INIT_CAPACITY * 3 + 123 - 1;i >= 0;i--) {
    long ret = (long)stack_pop(stack);
    assert(ret == i);
  }

  assert(stack->size == 0);
  assert(stack->capacity > STACK_INIT_CAPACITY * 3);

  stack_free(stack);
  printf("Pass!\n");

  return;
}

void test_get_op() {
  printf("=== Test token_get_op() ===\n");
  char *p;
  char test1[] = "-----====-=-=++==";
  token_t token;
  p = test1;
  while(*(p = token_get_op(p, &token)) != '\0') {
    printf("%d ", token.type);
  }
  putchar('\n');

  printf("Pass!\n");
  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_stack();
  test_get_op();
  return 0;
}