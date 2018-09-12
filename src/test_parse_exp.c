
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"
#include "error.h"
#include "ast.h"
#include "parse_exp.h"
#include "parse_decl.h"
#include "parse_comp.h"
#include "parse_stmt.h"
#include "hashtable.h"

void test_stack() {
  printf("=== Test Stack ===\n");
  stack_t *stack = stack_init();

  for(long i = 0;i < STACK_INIT_CAPACITY * 3 + 123;i++) {
    stack_push(stack, (void *)i);
  }

  for(long i = STACK_INIT_CAPACITY * 3 + 123 - 1;i >= 0;i--) {
    long ret = (long)stack_pop(stack);
    assert(ret == i);
  }

  assert(stack->size == 0);
  assert(stack->capacity > STACK_INIT_CAPACITY * 3);

  stack_free(stack);
  printf("Pass!\n");

  return;
}

void test_get_op() {
  printf("=== Test token_get_op() ===\n");
  char *p;
  char test1[] = "-----====-=-=++>>=>>>.+..+...+....+......";
  char result[256];
  token_t token;
  p = test1;
  result[0] = '\0';
  token_cxt_t *token_cxt = token_cxt_init(test1);
  while(p != NULL) {
    p = token_get_op(p, &token);
    if(p == NULL) break;
    else if(token.type != T_ILLEGAL) {
      printf("%s(%s) ", token_typestr(token.type), token_symstr(token.type));
      strcat(result, token_symstr(token.type));
    } else {
      p = token_get_ident(token_cxt, p, &token);
      if(p == NULL) break;
      else if(token.type != T_ILLEGAL) {
        printf("%s(%s) ", token_typestr(token.type), token.str);
        strcat(result, token.str);
        free(token.str);
      } else {
        assert(0);
      }
    }
  }
  putchar('\n');
  assert(strcmp(result, test1) == 0);
  token_cxt_free(token_cxt);

  printf("Pass!\n");
  return;
}

void test_bin_search() {
  printf("=== Test token_get_keyword_type() ===\n");
  token_type_t type;
  for(int i = 0;i < (int)sizeof(keywords) / (int)sizeof(const char *);i++) {
    type = token_get_keyword_type(keywords[i]);
    if(type == T_ILLEGAL) {
      printf("ILLEGAL %s\n", keywords[i]);
      assert(0);
    } else {
      printf("%s(%s) ", token_typestr(type), token_symstr(type));
      assert(strcmp(token_symstr(type), keywords[i]) == 0);
    }
  }

  type = token_get_keyword_type("aaaa");
  assert(type == T_ILLEGAL);
  type = token_get_keyword_type("zzzzzzz");
  assert(type == T_ILLEGAL);
  type = token_get_keyword_type("wangziqi");
  assert(type == T_ILLEGAL);
  type = token_get_keyword_type("jklasd");
  assert(type == T_ILLEGAL);

  putchar('\n');
  printf("Pass!\n");
  return;
}

void test_token_get_next() {
  printf("=== Test test_token_get_next() ===\n");
  char test[] = \
    "// Hello World \n \
     void main() {  \n \
        /* This is a block comment   \n \
           That cross multiple lines \n \
         */                          \n \
     }                               \n \
     \n";
  error_init(test);
  token_cxt_t *token_cxt = token_cxt_init(test);
  token_t *token;
  while((token = token_get_next(token_cxt)) != NULL) {
    const char *sym = token_symstr(token->type);
    if(sym == NULL) printf("%s ", token->str);
    else printf("%s ", sym);
    token_free(token);
  }
  putchar('\n');

  char test2[] = " \n \
    // Returns the next token, or illegal                           \n \
    // Same rule for return value and conditions as token_get_op()  \n \
    char *token_get_next(char *s, token_t *token) {                 \n \
      while(1) {                                                    \n \
        if(s == NULL || *s == '\\0') return NULL;                    \n \
        else if(isspace(*s)) while(isspace(*s)) s++;                \n \
        else if(s[0] == '/' && s[1] == '/') while(*s != '\\n' && *s != '\\0') s++; \n \
        else if(s[0] == '/' && s[1] == '*') {                         \n \
          while((s[0] != '\\0') && (s[0] != '*' || s[1] != '/')) s++;  \n \
          s += 2;                                                     \n \
        }                                                             \n \
        else if(isalpha(*s) || *s == '_') return token_get_ident(s, token); \n \
        else return token_get_op(s, token);                                 \n \
      }                                                                     \n \
                                                                            \n \
      assert(0);    \n \
      return NULL;  \n \
    }               \n \
  \" asda dasdasd\\n \" ";
  error_init(test2);
  while((token = token_get_next(token_cxt)) != NULL) {
    const char *sym = token_symstr(token->type);
    int row, col;
    error_get_row_col(token->offset, &row, &col);
    if(sym == NULL) printf("%s ", token->str);
    else printf("%s(%d %d) ", sym, row, col);
    token_free(token);
  }
  putchar('\n');
  token_cxt_free(token_cxt);

  printf("Pass!\n");
  return;
}

void test_ast() {
  printf("=== Test AST ===\n");
  // lvl | node content
  // [1] | 1 
  // [2] | 2 3   4 5
  // [3] |   6 7   8 
  // Should print 1 2 3 6 7 4 5 8
  token_t token1; token1.type = 1; token1.child = token1.sibling = NULL;
  token_t token2; token2.type = 2; token2.child = token2.sibling = NULL;
  token_t token3; token3.type = 3; token3.child = token3.sibling = NULL;
  token_t token4; token4.type = 4; token4.child = token4.sibling = NULL;
  token_t token5; token5.type = 5; token5.child = token5.sibling = NULL;
  token_t token6; token6.type = 6; token6.child = token6.sibling = NULL;
  token_t token7; token7.type = 7; token7.child = token7.sibling = NULL;
  token_t token8; token8.type = 8; token8.child = token8.sibling = NULL;
  ast_push_child(&token1, &token3);
  ast_push_child(&token1, &token2);
  ast_append_child(&token1, &token4);
  ast_append_child(&token1, &token5);
  ast_append_child(&token3, &token6);
  ast_append_child(&token3, &token7);
  ast_push_child(&token5, &token8);

  ast_print(&token1, 0);
  printf("Pass!\n");
  return;
}

void test_decl_prop() {
  printf("=== Test Declaration Property ===\n");
  char test1[] = "extern volatile unsigned const";    // unsigned
  char test2[] = "auto unsigned long long int const"; // unsigned long long int
  char test3[] = "signed short int extern";           // signed short int
  char test4[] = "extern unsigned long long const";   // unsigned long long
  char test5[] = "long double typedef";               // long double
  char test6[] = "volatile signed long int const";    // signed long int
  char test7[] = "volatile struct {int a, b : 50; signed long long int c;} const";    // struct
  char *tests[] = {test1, test2, test3, test4, test5, test6, test7, };
  for(int iter = 0;iter < (int)sizeof(tests) / (int)sizeof(char *);iter++) {
    char *s = tests[iter];
    parse_exp_cxt_t *cxt = parse_exp_init(s);
    printf("Iter #%d %s: \n", iter, s);
    token_t *basetype = parse_decl_basetype(cxt);
    assert(token_get_next(cxt->token_cxt) == NULL);
    printf("--> Reconstruct: %s\n", token_decl_print(basetype->decl_prop));
    if(iter == 6) ast_print(basetype, 0);
    ast_free(basetype);
    parse_exp_free(cxt);
  }
  
  printf("Pass!\n");
  return;
}

void test_token_lookahead() {
  printf("=== Test Token Lookahead ===\n");
  char test[] = "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16";
  token_cxt_t *token_cxt = token_cxt_init(test);
  token_t *token, *t1, *t2, *t3, *t4;
  t1 = token_get_next(token_cxt);
  t2 = token_get_next(token_cxt);
  t3 = token_get_next(token_cxt);
  t4 = token_get_next(token_cxt);
  token_pushback(token_cxt, t1);
  token_pushback(token_cxt, t2);
  token_pushback(token_cxt, t3);
  token_pushback(token_cxt, t4);
  assert(token_cxt->pb_num == 4);
  for(int i = 1;i <= 3;i++) { // Should see 1 2 3
    token = token_get_next(token_cxt);
    assert(atoi(token->str) == i);
    token_free(token);
  }
  for(int i = 1;i <= 5;i++) {  // Should see 4 5 6 7 8
    token = token_lookahead(token_cxt, i);
    assert(atoi(token->str) == i + 3);
  }
  for(int i = 1;i <= 5;i++) { // Should see 4 5 6 7 8 again
    token = token_get_next(token_cxt);
    assert(atoi(token->str) == i + 3);
    token_free(token);
  }
  for(int i = 1;i <= 5;i++) { // Should see 9 10 11 12 13
    token = token_lookahead(token_cxt, i);
    assert(atoi(token->str) == i + 8);
  }
  for(int i = 9;i <= 100;i++) { // Should see NULL .... but allocates 14 15 16
    token = token_lookahead(token_cxt, i);
    assert(token == NULL);
  }
  for(int i = 8;i >= 1;i--) { // Should see 16 15 14 13 12 11 10 9
    token = token_lookahead(token_cxt, i); 
    assert(atoi(token->str) == i + 8);
  }
  token_cxt_free(token_cxt); // Should free the rest of the token nodes (9 - 16)
  printf("Pass!\n");
  return;
}

void test_ht() {
  printf("=== Test Hash Table ===\n");
  const int test_size = HT_INIT_CAPACITY * 10 + 100;
  const int test_len = 16;
  const char alphabet[] = {"qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM_"};
  for(int seed = 0;seed < 100;seed++) {
    srand(seed);
    char **tests = malloc(sizeof(char *) * test_size);
    void **results = malloc(sizeof(void *) * test_size);
    for(int i = 0;i < test_size;i++) {
      tests[i] = malloc(sizeof(char) * test_len);
      for(int j = 0;j < test_len - 1;j++) tests[i][j] = alphabet[rand() % sizeof(alphabet)];
      tests[i][test_len - 1] = '\0';
    }

    hashtable_t *ht = ht_str_init();
    for(int i = 0;i < test_size;i++) {
      results[i] = ht_insert(ht, tests[i], tests[i]);
    }
    printf("Finished: %06d [ Size: %d; Capacity: %d ]\r", seed, ht->size, ht->capacity);
    for(int i = test_size - 1;i >= 0;i--) {
      void *ret = ht_find(ht, tests[i]);
      assert(ret != HT_NOTFOUND);
      assert(strcmp(ret, tests[i]) == 0);
    }

    assert(ht_find(ht, "wangziqi2013") == HT_NOTFOUND);
    assert(ht_find(ht, "+_1234567890") == HT_NOTFOUND);
    assert(ht_find(ht, "!@#$") == HT_NOTFOUND);
    assert(ht_find(ht, "QWERT[]{}") == HT_NOTFOUND);

    for(int i = 0;i < test_size;i++) free(tests[i]);
    free(tests);
    free(results);
    ht_free(ht);
  }
  printf("\nPass!\n");
  return;
}

void test_simple_exp_parse() {
  printf("=== Test Simple Expression Parsing ===\n");
  char test[] = " g(*a[0]++) + ((f(1,2,3,((wzq123 + 888)--)))) * (a++ >> b + ++c * ***d[++wzq--[1234]])";
  parse_exp_cxt_t *cxt = parse_exp_init(test);
  token_t *token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "x == x + 2 && qwe > rty ? (void const volatile *const volatile*const*volatile[*zaq + qwer--])y * 6 >> 3 : *z++ += 1000";
  cxt = parse_exp_init(test2);
  token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "(void **(int **, long, short))g + (void* (*named_decl[16]) (void a, int *[]) ) a()++ - sizeof(1) * sizeof **a++";
  cxt = parse_exp_init(test3);
  token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test4[] = "a[b++]";
  cxt = parse_exp_init(test4);
  token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test5[] = "ARRAY[(a, b), c], d";  // Tests whether the outer most comma is rejected
  cxt = parse_exp_init(test5);
  token = parse_exp(cxt, PARSE_EXP_NOCOMMA);
  assert(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_COMMA);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("Pass!\n");
  return;
}

void test_parse_decl() {
  printf("=== Test parse_decl ===\n");
  parse_exp_cxt_t *cxt;
  token_t *token;
  char test1[] = "void const * const ( *const named_decl[16]) (void a, int *[])";
  cxt = parse_exp_init(test1);
  token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "void a ";
  cxt = parse_exp_init(test2);
  token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "a(void, int[16])";
  cxt = parse_exp_init(test3);
  token = parse_decl(cxt, PARSE_DECL_NOBASETYPE);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("Pass!\n");
  return;
}

void test_parse_struct_union() {
  printf("=== Test parse_struct_union ===\n");
  parse_exp_cxt_t *cxt;
  token_t *token;
  char test1[] = "struct a { int b; long c; volatile double d; }";
  cxt = parse_exp_init(test1);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "struct { void : 50, **aa : 100, []; int bb : 20; long; } ";
  cxt = parse_exp_init(test2);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "struct {}";
  cxt = parse_exp_init(test3);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n"); // Tests nesting of struct and union
  char test4[] = "struct { struct{ int a; }; union { long b; }; }";
  cxt = parse_exp_init(test4);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n"); // Tests whether anonymous struct/union is allowed
  char test5[] = "struct name;";
  cxt = parse_exp_init(test5);
  token = parse_comp(cxt);
  assert(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_SEMICOLON);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("Pass!\n");
  return;
}

void test_parse_enum() {
  printf("=== Test parse_enum ===\n");
  parse_exp_cxt_t *cxt;
  token_t *token;
  char test1[] = "enum abcdefg";
  cxt = parse_exp_init(test1);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "enum {}";
  cxt = parse_exp_init(test2);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "enum {a,b,c,d,}";
  cxt = parse_exp_init(test3);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test4[] = "enum {a=1,b=2,c,d,e=5,}";
  cxt = parse_exp_init(test4);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test5[] = "enum name {a = (0,1,2), b = a ? 100 : 200, c = b}";
  cxt = parse_exp_init(test5);
  token = parse_comp(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("Pass!\n");
  return;
}

// This test may introduce memory leak
void test_anomaly() {
  printf("=== Test anomalies ===\n");
  error_testmode(1);
  int err;  
  parse_exp_cxt_t *cxt;
  char test1[] = "d[++wzq--[1234]";  // Tests if [ and ( must be balanced
  err = 0;
  cxt = parse_exp_init(test1);
  if(error_trycatch()) parse_exp(cxt, PARSE_EXP_ALLOWALL);
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);

  char test2[] = "(***a(b*)(void))";
  err = 0;
  cxt = parse_exp_init(test2);
  if(error_trycatch()) parse_decl(cxt, 1);
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);
  
  error_testmode(0);
  printf("Pass!\n");
  return;
}

void test_parse_stmt() {
  printf("=== Test parse_stmt ===\n");
  parse_exp_cxt_t *cxt;
  token_t *token;
  char test1[] = "case (1 == 2 ? 2 : 4): break;"; // First colon should be parsed as expression
  cxt = parse_exp_init(test1);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "label_2: continue;";
  cxt = parse_exp_init(test2);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "default: break;";
  cxt = parse_exp_init(test3);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test4[] = "return;";
  cxt = parse_exp_init(test4);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test5[] = "return 1 ? 2 : 3 + 4 **5;";
  cxt = parse_exp_init(test5);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test6[] = "goto label1;";
  cxt = parse_exp_init(test6);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test7[] = "a + b * c << d, e, f;";
  cxt = parse_exp_init(test7);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test8[] = "{1, 4, {5, {a + b ? c : d, (10, 11), }, 7, {} }, {}}"; // {,} is invalid
  cxt = parse_exp_init(test8);
  token = parse_init_list(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  printf("Pass!\n");
  return;
}

void test_parse_comp_stmt() {
  printf("=== Test parse_comp_stmt ===\n");
  parse_exp_cxt_t *cxt;
  token_t *token;
  char test1[] = "{int a, b, c; void **d = NULL, (*e)() = NULL; }"; // Test multiple variables
  cxt = parse_exp_init(test1);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "{ int a[10][20] = {{1,2,3}, {4,}, {5, 6, 7}}; a[0][1] = 100; }"; // Test init list
  cxt = parse_exp_init(test2);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "{}"; // Test empty block
  cxt = parse_exp_init(test3);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n"); 
  char test4[] = "{ a = b; c = d; return a == c; }"; // Test empty var decl
  cxt = parse_exp_init(test4);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  printf("Pass!\n");
  return;
}

void test_parse_select_stmt() {
  printf("=== Test parse_select_stmt ===\n");
  parse_exp_cxt_t *cxt;
  token_t *token;
  char test1[] = "if(a == b) x = y; else { x != y; }"; // Test if else with comp stmt
  cxt = parse_exp_init(test1);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "if(a == b) x; else if(c == d) { second_if; } else not_block;"; // nested if in else stmt
  cxt = parse_exp_init(test2);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "if(a == b) if(c == d) inner_if; else inner_else; else outer_else;"; // nested if in if stmt
  cxt = parse_exp_init(test3);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n"); 
  char test4[] = "switch(a == b) { a = b; switch(1) return; c = d; return a == c; }"; // Test empty var decl
  cxt = parse_exp_init(test4);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  printf("Pass!\n");
  return;
}

void test_parse_loop_stmt() {
  printf("=== Test parse_loop_stmt ===\n");
  parse_exp_cxt_t *cxt;
  token_t *token;
  char test1[] = "while(a == b) return;";
  cxt = parse_exp_init(test1);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "while(a == b) { c = d + e; return; }";
  cxt = parse_exp_init(test2);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "do a = b; while(1 == 2);"; 
  cxt = parse_exp_init(test3);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n"); 
  char test4[] = "do { int a = b, c; return; } while(d ? e : f);"; 
  cxt = parse_exp_init(test4);
  token = parse_stmt(cxt);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  printf("Pass!\n");
  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_stack();
  test_get_op();
  test_bin_search();
  test_token_get_next();
  test_ast();
  test_ht();
  test_decl_prop();
  test_token_lookahead();
  test_simple_exp_parse();
  test_parse_decl();
  test_parse_struct_union();
  test_parse_enum();
  test_parse_stmt();
  test_parse_comp_stmt();
  test_parse_select_stmt();
  test_parse_loop_stmt();
  //test_anomaly();
  return 0;
}