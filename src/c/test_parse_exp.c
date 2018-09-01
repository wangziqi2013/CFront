
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"

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
  char test1[] = "-----====-=wangziqi2013-=++>>=>>>_____ident";
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
  printf("=== Test get_keyword_type() ===\n");
  token_type_t type;
  for(int i = 0;i < sizeof(keywords) / sizeof(const char *);i++) {
    type = get_keyword_type(keywords[i]);
    if(type == T_ILLEGAL) {
      printf("ILLEGAL %s\n", keywords[i]);
      assert(0);
    } else {
      printf("%s(%s) ", token_typestr(type), token_symstr(type));
      assert(strcmp(token_symstr(type), keywords[i]) == 0);
    }
  }

  type = get_keyword_type("aaaa");
  assert(type == T_ILLEGAL);
  type = get_keyword_type("zzzzzzz");
  assert(type == T_ILLEGAL);
  type = get_keyword_type("wangziqi");
  assert(type == T_ILLEGAL);
  type = get_keyword_type("jklasd");
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
  while((s = token_get_next(s, &token)) != NULL) {
    const char *sym = token_symstr(token.type);
    if(sym == NULL) printf("%s ", token.str);
    else printf("%s ", token_symstr(token.type));
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
  ";
  s = test2;
  while((s = token_get_next(s, &token)) != NULL) {
    const char *sym = token_symstr(token.type);
    if(sym == NULL) printf("%s ", token.str);
    else printf("%s ", token_symstr(token.type));
  }
  putchar('\n');

  printf("Pass!\n");
  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_stack();
  test_get_op();
  test_bin_search();
  test_token_get_next();
  return 0;
}