
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"
#include "error.h"
#include "ast.h"
#include "parse.h"
#include "hashtable.h"
#include "type.h"

void test_scope_init() {
  printf("=== Test Scope Init ===\n");
  printf("Pass!\n");
  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_scope_init();
  return 0;
}
  