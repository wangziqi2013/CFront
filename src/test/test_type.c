
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
  type_cxt_t *cxt = type_init();
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2013", (void *)0x12345UL);
  scope_top_insert(cxt, SCOPE_UNION, "wangziqi2016", (void *)0x23456UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_recurse(cxt); // 2 levels
  assert(scope_top_find(cxt, SCOPE_STRUCT, "wangziqi2013") == HT_NOTFOUND);
  assert(scope_top_find(cxt, SCOPE_UNION, "wangziqi2016") == HT_NOTFOUND);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_recurse(cxt); // 3 levels
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_decurse(cxt); // 2 levels
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_recurse(cxt); // 3 levels
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2013", (void *)0x34567UL);
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2018", (void *)0x45678UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x34567UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2018") == (void *)0x45678UL);
  scope_recurse(cxt);
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2013", (void *)0x56789UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x56789UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2018") == (void *)0x45678UL);
  type_free(cxt);
  printf("Pass!\n");
  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_scope_init();
  return 0;
}
  