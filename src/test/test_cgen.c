
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

typedef struct {
  cgen_cxt_t *cgen_cxt;       // This contains type cxt
  type_cxt_t *type_cxt;
  parse_exp_cxt_t *parse_cxt; // This contains token cxt
  token_cxt_t *token_cxt;
} test_cxt_t;

tets_cxt_t *test_init(const char *s) {
  test_cxt_t *cxt = (test_cxt_t *)malloc(sizeof(test_cxt_t));
  SYSEXPECT(cxt != NULL);
  memset(cxt, 0x00, sizeof(test_cxt_t));
  cxt->cgen_cxt = cgen_init();
  cxt->type_cxt = cxt->cgen_cxt->type_cxt;
  cxt->parse_cxt = parse_exp_init(s);
  cxt->token_cxt = cxt->parse_cxt->token_cxt;
  return cxt;
}

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