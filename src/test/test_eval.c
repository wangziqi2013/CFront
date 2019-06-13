
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"
#include "error.h"
#include "ast.h"
#include "parse.h"
#include "hashtable.h"
#include "bintree.h"
#include "list.h"
#include "str.h"
#include "type.h"
#include "eval.h"

void test_const_eval_int() {
  printf("=== Test const_eval_int ===\n");
  parse_exp_cxt_t *parse_cxt;
  type_cxt_t *type_cxt;
  token_t *token;
  type_t *type;
  value_t *value;
  
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("123456");
  token = token_get_next(parse_cxt->token_cxt);
  assert(token);
  value = eval_const_get_int_value(type_cxt, token);
  printf("Type: %s Value: 0x%016lX", type_print_str(0, value->type, NULL, 0), value->uint64);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");

  printf("Pass!\n");
  return;
}

int main() {
  test_const_eval_int();
  return 0;
}