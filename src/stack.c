
#include "stack.h"

stack_t *stack_init() {
  stack_t *stack = (stack_t *)malloc(sizeof(stack_t));
  if(stack == NULL) syserror(__func__);
  stack->data = (void **)malloc(sizeof(void *) * STACK_INIT_CAPACITY);
  if(stack->data == NULL) syserror(__func__);
  stack->size = 0;
  stack->capacity = STACK_INIT_CAPACITY;

  return stack;
}

void stack_free(stack_t *stack) {
  free(stack->data);
  free(stack);
  return;
}

void stack_push(stack_t *stack, void *p) {
  if(stack->size == stack->capacity) {
    void **old = stack->data;
    stack->data = malloc(sizeof(void *) * stack->capacity * 2);
    if(stack->data == NULL) syserror(__func__);
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
  return stack->data[--stack->size];
}

void *stack_peek(stack_t *stack) {
  assert(stack->size != 0);
  return stack->data[stack->size - 1];
}

// Offset is from the top of the stack towards the bottom
void *stack_peek_at(stack_t *stack, int offset) {
  assert(offset >= 0 && offset < stack->size);
  return stack->data[stack->size - 1 - offset];
}

void *stack_at(stack_t *stack, int index) {
  assert(index >= 0 && index < stack->size);
  return stack->data[index];
}

void **stack_topaddr(stack_t *stack) {
  return stack->data + stack->size;
}

int stack_empty(stack_t *stack) { return stack->size == 0; }
int stack_size(stack_t *stack) { return stack->size; }
