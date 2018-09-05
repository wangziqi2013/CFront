
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"
#include "error.h"
#include "ast.h"
#include "parse_exp.h"
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
  for(int i = 0;i < sizeof(keywords) / sizeof(const char *);i++) {
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
  char *s = test;
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
  s = test2;
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

void test_simple_exp_parse() {
  printf("=== Test Simple Expression Parsing ===\n");
  char test[] = " g(*a[0]++) + ((f(1,2,3,((wzq123 + 888)--)))) * (a++ >> b + ++c * ***d[++wzq--[1234]])";
  parse_exp_cxt_t *cxt = parse_exp_init(test);
  token_t *token = parse_exp(cxt);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "x == x + 2 && qwe > rty ? y * 6 >> 3 : *z++ += 1000";
  cxt = parse_exp_init(test2);
  token = parse_exp(cxt);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "sizeof(1) * sizeof **a++";
  cxt = parse_exp_init(test3);
  token = parse_exp(cxt);
  ast_print(token, 0);
  parse_exp_free(cxt);
  ast_free(token);
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

void test_decl_prop() {
  printf("=== Test Declaration Property ===\n");
  char test1[] = "extern volatile const int unsigned";
  char test2[] = "auto const unsigned float";
  char test3[] = "signed char int";
  char test4[] = "unsigned extern const long long"; // do not support long long type
  char test5[] = "long double typedef"; // do not support long double type
  char test6[] = "volatile const const";
  char *tests[] = {test1, test2, test3, test4, test5, test6, };
  for(int iter = 0;iter < sizeof(tests) / sizeof(char *);iter++) {
    decl_prop_t decl_prop = DECL_NULL;
    char *s = tests[iter];
    token_cxt_t *token_cxt = token_cxt_init(s);
    printf("Iter #%d %s: \n", iter, s);
    error_init(s);
    int comp = 1;
    token_t *token;
    while((token = token_get_next(token_cxt)) != NULL) {
      decl_prop_t new_decl_prop = token_decl_apply(token, decl_prop);
      if(new_decl_prop == DECL_INVALID) {
        printf("--> Incompatible: %s and %s\n", token_typestr(token->type), token_decl_print(decl_prop));
        token_free(token);
        comp = 0;
        break;
      }
      else decl_prop = new_decl_prop;
      token_free(token);
    }
    if(comp) printf("--> Reconstruct: %s\n", token_decl_print(decl_prop));
    token_cxt_free(token_cxt);
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
  assert(atoi(token_get_next(token_cxt)->str) == 1);
  assert(atoi(token_get_next(token_cxt)->str) == 2);
  assert(atoi(token_get_next(token_cxt)->str) == 3);
  for(int i = 1;i <= 5;i++) {  // Should see 4 5 6 7 8
    token = token_lookahead(token_cxt, i);
    assert(atoi(token->str) == i + 3);
  }
  for(int i = 1;i <= 5;i++) { // Should see 4 5 6 7 8 again
    token = token_get_next(token_cxt);
    assert(atoi(token->str) == i + 3);
  }
  for(int i = 1;i <= 5;i++) { // Should see 9 10 11 12 13
    token = token_lookahead(token_cxt, i);
    assert(atoi(token->str) == i + 8);
  }
  for(int i = 9;i <= 100;i++) { // Should see NULL ....
    token = token_lookahead(token_cxt, i);
    assert(token == NULL);
  }
  for(int i = 8;i >= 1;i--) { // Should see 13 12 11 10 9
    token = token_lookahead(token_cxt, i);
    assert(atoi(token->str) == i + 8);
  }
  token_cxt_free(token_cxt);
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
  test_simple_exp_parse();
  test_ht();
  test_decl_prop();
  test_token_lookahead();
  return 0;
}