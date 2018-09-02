
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"
#include "error.h"
#include "ast.h"
#include "parse_exp.h"

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
  char test1[] = "-----====-=-=++>>=>>>";
  char result[256];
  token_t token;
  p = test1;
  result[0] = '\0';
  while(p != NULL) {
    p = token_get_op(p, &token);
    if(p == NULL) break;
    else if(token.type != T_ILLEGAL) {
      printf("%s(%s) ", token_typestr(token.type), token_symstr(token.type));
      strcat(result, token_symstr(token.type));
    } else {
      p = token_get_ident(p, &token);
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
  token_t token;
  char *s = test;
  error_init(test);
  while((s = token_get_next(s, &token)) != NULL) {
    const char *sym = token_symstr(token.type);
    if(sym == NULL) printf("%s ", token.str);
    else printf("%s ", sym);
    token_free_literal(&token);
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
  while((s = token_get_next(s, &token)) != NULL) {
    const char *sym = token_symstr(token.type);
    int row, col;
    error_get_row_col(token.offset, &row, &col);
    if(sym == NULL) printf("%s ", token.str);
    else printf("%s(%d %d) ", sym, row, col);
    token_free_literal(&token);
  }
  putchar('\n');

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
  char test[] = "a++ >> b + ++c * *d";
  parse_exp_cxt_t *cxt = parse_exp_init(test);
  token_t *token = parse_exp(cxt);
  ast_print(token, 0);
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
  return 0;
}