
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
  printf("=== Test eval_const_get_int_value ===\n");
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
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("0x80000000"); // This will be signed overflow
  token = token_get_next(parse_cxt->token_cxt);
  assert(token);
  value = eval_const_get_int_value(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("0x80000000U"); // This will be unsigned, no overflow
  token = token_get_next(parse_cxt->token_cxt);
  assert(token);
  value = eval_const_get_int_value(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("0xFFFFFFFFU"); // This will be unsigned, no overflow
  token = token_get_next(parse_cxt->token_cxt);
  assert(token);
  value = eval_const_get_int_value(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("0x1FFFFFFFFU"); // This will be unsigned, overflow
  token = token_get_next(parse_cxt->token_cxt);
  assert(token);
  value = eval_const_get_int_value(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("'\xfe'"); // Although it overflows for char type, it is evaluated by another function, no warning
  token = token_get_next(parse_cxt->token_cxt);
  assert(token);
  value = eval_const_get_int_value(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");

  printf("Pass!\n");
  return;
}

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
  ast_print_(token, 0);
  value = eval_const_exp(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  assert(value->int32 == 16096);
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("((char)1000 + 2ul * 3) << 4"); 
  token = parse_exp(parse_cxt, PARSE_EXP_ALLOWALL);
  ast_print_(token, 0);
  value = eval_const_exp(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  assert(value->int64 == -288); // Because of the sign extension of char type
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  type_cxt = type_sys_init();
  parse_cxt = parse_exp_init("(signed long)((long)(unsigned long *)(long)(unsigned long *)(long)100 + 2)"); 
  token = parse_exp(parse_cxt, PARSE_EXP_ALLOWALL);
  ast_print_(token, 0);
  value = eval_const_exp(type_cxt, token);
  printf("Type: %s Value: 0x%016lX (%ld)\n", type_print_str(0, value->type, NULL, 0), value->uint64, value->int64);
  assert(value->int64 == 102); // Because of the sign extension of char type
  parse_exp_free(parse_cxt);
  type_sys_free(type_cxt);
  printf("=====================================\n");

  printf("Pass!\n");
  return;
}

int main() {
  test_const_eval_int();
  test_eval_const_exp();
  return 0;
}