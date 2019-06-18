
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

test_cxt_t *test_init(char *s) {
  test_cxt_t *cxt = (test_cxt_t *)malloc(sizeof(test_cxt_t));
  SYSEXPECT(cxt != NULL);
  memset(cxt, 0x00, sizeof(test_cxt_t));
  cxt->cgen_cxt = cgen_init();
  cxt->type_cxt = cxt->cgen_cxt->type_cxt;
  cxt->parse_cxt = parse_exp_init(s);
  cxt->token_cxt = cxt->parse_cxt->token_cxt;
  return cxt;
}

void test_free(test_cxt_t *cxt) {
  cgen_free(cxt->cgen_cxt);
  parse_exp_free(cxt->parse_cxt);
  free(cxt);
  return;
}

void test_cgen_global_decl() {
  printf("=== Test cgen_global_decl ===\n");

  test_cxt_t *cxt;
  token_t *token;

  cxt = test_init("extern const int array[120 + 20];");
  token = parse(cxt->parse_cxt);
  ast_print(token);
  ast_free(token);
  test_free(cxt);
  printf("=====================================\n");
  

  printf("Pass!\n");
  return;
}


int main() {
  printf("Hello World!\n");
  test_cgen_global_decl();
  return 0;
}