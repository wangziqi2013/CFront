
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
#include "cgen.h"

/* 
void test_eval_const_exp() {
  printf("=== Test eval_const_exp ===\n");
  parse_exp_cxt_t *parse_cxt;
  type_cxt_t *type_cxt;
  token_t *token;
  type_t *type;
  value_t *value;

  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("(1000 + 2 * 3) << 4"); 
  token = parse_exp(parse_cxt, PARSE_EXP_ALLOWALL);
  ast_print(token, 0);
  value = eval_const_exp(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  assert(value->int32 == 16096);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  

  printf("Pass!\n");
  return;
}
*/

int main() {
  printf("Hello World!\n");
  return 0;
}