
#include "token.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define STACK_INIT_CAPACITY 128

// Implements a general stack which is used in the shift-reduce parsing algo.
typedef struct {
  int size;
  int capacity;
  void **data;
} stack_t;



void stack_push(stack_t *stack, void *p) {
  if(stack->size == stack->capacity) {
    void **old = stack->data;
    stack->data = malloc(sizeof(void *) * stack->capacity * 2);
    if(stack->data == NULL) perror(__func__);
    memcpy(stack->data, old, sizeof(void *) * stack->capacity);
    stack->capacity *= 2;
    free(old);
  }
  assert(stack->size < stack->capacity);
  stack->data[stack->size++] = p;
  return;
}

void *stack_pop(stack_t *stack) {
  assert(stack->size != 0);
  return stack->data[--size];
}