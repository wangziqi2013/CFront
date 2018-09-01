
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
  char test1[] = "-----====-=wangziqi2013-=++>>=>>>_____ident";
  char result[256];
  token_t token;
  p = test1;
  result[0] = '\0';
  while(p != NULL) {
    p = token_get_op(p, &token);
    if(p == NULL) break;
    else if(token.type != T_ILLEGAL) {
      printf("%s(%s) ", token_typestr(token.type), token_symstr(token.type));
      strcat(result, token_symstr(token.type));
    } else {
      p = token_get_ident(p, &token);
      if(p == NULL) break;
      else if(token.type != T_ILLEGAL) {
        printf("%s(%s) ", token_typestr(token.type), token.str);
        strcat(result, token.str);
        free(token.str);
      } else {
        assert(0);
      }
    }
  }
  putchar('\n');
  assert(strcmp(result, test1) == 0);

  printf("Pass!\n");
  return;
}

void test_bin_search() {
  
}

int main() {
  printf("=== Hello World! ===\n");
  test_stack();
  test_get_op();
  return 0;
}