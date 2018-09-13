
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "token.h"
#include "error.h"
#include "ast.h"
#include "parse.h"
#include "hashtable.h"

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

void test_int_size() {
  printf("=== Test Integer Size ===\n");
  char test[] = "12 23l 34ll 45llu 56lu 67u 78ul 89ull";
  token_cxt_t *cxt = token_cxt_init(test);
  token_t *token;
  while((token = token_get_next(cxt)) != NULL) {
    printf("%s %s\n", token->str, token_decl_print(token->decl_prop));
  }
  token_cxt_free(cxt);
  printf("Pass!\n");
  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_get_op();
  test_bin_search();
  test_token_get_next();
  test_int_size();
  return 0;
}
  