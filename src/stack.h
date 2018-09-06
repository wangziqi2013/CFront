
#ifndef _STACK_H
#define _STACK_H

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

stack_t *stack_init();
void stack_free(stack_t *stack);
void stack_push(stack_t *stack, void *p);
void *stack_pop(stack_t *stack);
void *stack_peek(stack_t *stack);
void *stack_peek_at(stack_t *stack, int offset);
void *stack_at(stack_t *stack, int index);
int stack_empty(stack_t *stack);
int stack_size(stack_t *stack);
void **stack_topaddr(stack_t *stack);

#endif