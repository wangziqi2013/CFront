
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

void test_scope_init() {
  printf("=== Test Scope Init ===\n");
  type_cxt_t *cxt = type_sys_init();
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2013", (void *)0x12345UL);
  scope_top_insert(cxt, SCOPE_UNION, "wangziqi2016", (void *)0x23456UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_recurse(cxt); // 2 levels
  assert(!scope_top_find(cxt, SCOPE_STRUCT, "wangziqi2013"));
  assert(!scope_top_find(cxt, SCOPE_UNION, "wangziqi2016"));
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_recurse(cxt); // 3 levels
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_decurse(cxt); // 2 levels
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_recurse(cxt); // 3 levels
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2013", (void *)0x34567UL);
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2018", (void *)0x45678UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x34567UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2018") == (void *)0x45678UL);
  scope_recurse(cxt);
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2013", (void *)0x56789UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x56789UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2018") == (void *)0x45678UL);
  type_sys_free(cxt);
  printf("Pass!\n");
  return;
}

void test_bintree() {
  printf("=== Test Binary Tree ===\n");
  const int test_size = 128 * 10 + 100;
  const int test_len = 16;
  const char alphabet[] = {"qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM_"};
  for(int seed = 0;seed < 100;seed++) {
    srand(seed);
    char **tests = malloc(sizeof(char *) * test_size);
    void **results = malloc(sizeof(void *) * test_size);
    for(int i = 0;i < test_size;i++) {
      tests[i] = malloc(sizeof(char) * test_len);
      for(int j = 0;j < test_len - 1;j++) tests[i][j] = alphabet[rand() % (sizeof(alphabet) - 1)]; // Do not allow '\0'
      tests[i][test_len - 1] = '\0';
    }

    bintree_t *bt = bt_str_init();
    for(int i = 0;i < test_size;i++) {
      results[i] = bt_insert(bt, tests[i], tests[i]);
    }
    
    for(int i = test_size - 1;i >= 0;i--) {
      void *ret = bt_find(bt, tests[i]);
      assert(ret != BT_NOTFOUND);
      assert(strcmp(ret, tests[i]) == 0);
    }

    assert(bt_find(bt, "wangziqi2013") == BT_NOTFOUND);
    assert(bt_find(bt, "+_1234567890") == BT_NOTFOUND);
    assert(bt_find(bt, "!@#$") == BT_NOTFOUND);
    assert(bt_find(bt, "QWERT[]{}") == BT_NOTFOUND);
    printf("Finished: %06d [ Size: %d ]\r", seed, bt_size(bt));

    for(int i = 0;i < test_size / 2;i++) {
      assert(bt_remove(bt, tests[i]) == tests[i]);
      assert(bt_find(bt, tests[i]) == BT_NOTFOUND);
      assert(bt_remove(bt, tests[i]) == BT_NOTFOUND);
    }
    for(int i = test_size - 1;i >= test_size / 2;i--) {
      assert(bt_remove(bt, tests[i]) == tests[i]);
      assert(bt_find(bt, tests[i]) == BT_NOTFOUND);
      assert(bt_remove(bt, tests[i]) == BT_NOTFOUND);
    }
    assert(bt_size(bt) == 0);
    assert(bt->root == NULL);

    for(int i = 0;i < test_size;i++) free(tests[i]);
    free(tests);
    free(results);
    bt_free(bt);
  }
  printf("\nPass!\n");
  return;
}

void test_list() {
  printf("=== Test Linked List ===\n");
  const int test_size = 128 * 2;  // Make it smaller since linked list is slow
  const int test_len = 16;
  const char alphabet[] = {"qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM_"};
  for(int seed = 0;seed < 100;seed++) {
    srand(seed);
    char **tests = malloc(sizeof(char *) * test_size);
    void **results = malloc(sizeof(void *) * test_size);
    for(int i = 0;i < test_size;i++) {
      tests[i] = malloc(sizeof(char) * test_len);
      for(int j = 0;j < test_len - 1;j++) tests[i][j] = alphabet[rand() % (sizeof(alphabet) - 1)]; // Do not allow '\0'
      tests[i][test_len - 1] = '\0';
    }

    list_t *list = list_init();
    for(int i = 0;i < test_size;i++) {
      results[i] = list_insert_nodup(list, tests[i], tests[i], streq_cb);
    }
    
    for(int i = test_size - 1;i >= 0;i--) {
      void *ret = list_find(list, tests[i], streq_cb);
      assert(ret != HT_NOTFOUND);
      assert(strcmp(ret, tests[i]) == 0);
    }

    assert(list_find(list, "wangziqi2013", streq_cb) == HT_NOTFOUND);
    assert(list_find(list, "+_1234567890", streq_cb) == HT_NOTFOUND);
    assert(list_find(list, "!@#$", streq_cb) == HT_NOTFOUND);
    assert(list_find(list, "QWERT[]{}", streq_cb) == HT_NOTFOUND);
    printf("Finished: %06d [ Size: %d ]\r", seed, list_size(list));
    //assert(list_size(list) == test_size);
    int mid1 = test_size / 4;
    int mid2 = mid1 * 2;
    int mid3 = mid1 * 3;
    for(int i = mid1;i < mid2;i++) {  // Ascending removal from middle
      assert(list_remove(list, tests[i], streq_cb) == tests[i]);
      assert(list_find(list, tests[i], streq_cb) == BT_NOTFOUND);
      assert(list_remove(list, tests[i], streq_cb) == BT_NOTFOUND);
    }
    for(int i = mid3 - 1;i >= mid2;i--) {  // Decending removal from middle
      assert(list_remove(list, tests[i], streq_cb) == tests[i]);
      assert(list_find(list, tests[i], streq_cb) == BT_NOTFOUND);
      assert(list_remove(list, tests[i], streq_cb) == BT_NOTFOUND);
    }
    for(int i = 0;i < mid1;i++) { // Remove from beginning
      assert(list_remove(list, tests[i], streq_cb) == tests[i]);
      assert(list_find(list, tests[i], streq_cb) == BT_NOTFOUND);
      assert(list_remove(list, tests[i], streq_cb) == BT_NOTFOUND);
    }
    for(int i = test_size - 1;i >= mid3;i--) { // Remove from end
      assert(list_remove(list, tests[i], streq_cb) == tests[i]);
      assert(list_find(list, tests[i], streq_cb) == BT_NOTFOUND);
      assert(list_remove(list, tests[i], streq_cb) == BT_NOTFOUND);
    }
    assert(list_size(list) == 0);
    assert(list->head == list->tail && list->head == NULL);

    for(int i = 0;i < test_size;i++) free(tests[i]);
    free(tests);
    free(results);
    list_free(list);
  }
  putchar('\n');
  list_t *list = list_init();
  list_insertat(list, "a", NULL, 0); // size 1  a   
  list_insertat(list, "b", NULL, 0); // size 2  b a
  list_insertat(list, "c", NULL, 1); // size 3  b c a
  list_insertat(list, "d", NULL, 2); // size 4  b c d a
  list_insertat(list, "e", NULL, 4); // size 5  b c d a e
  list_insertat(list, "f", NULL, 2); // size 6  b c f d a e
  list_insertat(list, "g", NULL, 5); // size 7  b c f d a g e
  // Should print: b c f d a g e
  char expected[] = "bcfdage", *p = expected;
  for(listnode_t *curr = list->head;curr;curr = curr->next) {
    printf("%s ", (char *)curr->key);
    assert(*(char *)curr->key == *p++);
  }
  putchar('\n');
  // Should print the same thing
  for(int i = 0;i < list_size(list);i++) {
    const listnode_t *curr = list_findat(list, i);
    printf("%s ", (char *)curr->key);
    assert(*(char *)curr->key == expected[i]);
  }
  assert(list_findat(list, list_size(list)) == LIST_NOTFOUND);
  putchar('\n');
  // Then test remove at
  void *key;
  list_removeat(list, 5, &key); printf("%c ", *(char *)key); assert(*(char *)key == 'g');
  list_removeat(list, 2, &key); printf("%c ", *(char *)key); assert(*(char *)key == 'f'); 
  list_removeat(list, 4, &key); printf("%c ", *(char *)key); assert(*(char *)key == 'e');
  list_removeat(list, 2, &key); printf("%c ", *(char *)key); assert(*(char *)key == 'd');
  list_removeat(list, 1, &key); printf("%c ", *(char *)key); assert(*(char *)key == 'c');
  list_removeat(list, 0, &key); printf("%c ", *(char *)key); assert(*(char *)key == 'b');
  list_removeat(list, 0, &key); printf("%c ", *(char *)key); assert(*(char *)key == 'a');
  assert(list_size(list) == 0);
  assert(list->head == NULL && list->tail == NULL);
  putchar('\n');
  list_free(list);
  printf("Pass!\n");
  return;
}

void test_str() {
  printf("=== Test str_t ===\n");
  str_t *s = str_init();
  char ch = 'a';
  while(ch != 'z') str_append(s, ch++);
  ch = 'A';
  while(ch != 'Z') str_append(s, ch++);
  for(int i = 0;i < 20;i++) str_concat(s, "wangziqi2013");
  char *s2 = str_copy(s);
  assert(strcmp(s2, s->s) == 0);
  puts(s2);
  str_free(s);
  free(s2);
  printf("Pass!\n");
  return;
}

void test_vector() {
  printf("=== Test vector ===\n");
  vector_t *vector = vector_init();
  const int test_size = VECTOR_INIT_SIZE * 10 + 13;
  for(int i = 0;i < test_size;i++) {
    vector_append(vector, (void *)(unsigned long)i);
  }
  for(int i = 0;i < test_size;i++) {
    assert(vector_at(vector, i) == (void *)(unsigned long)i);
  }
  assert(vector_size(vector) == test_size);
  vector_free(vector);
  printf("Pass!\n");
}

void test_eval_const_int() {
  printf("=== Test eval_const_int() ===\n");
  parse_exp_cxt_t *cxt;
  type_cxt_t *type_cxt = type_sys_init();
  token_t *token;
  char test1[] = "1 + 2 * 3 + 6 / 2";
  cxt = parse_exp_init(test1);
  token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  printf("Eval = %d\n", eval_const_int(type_cxt, token));
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "((0x10 << 16) >> 4) | 0x2345";
  cxt = parse_exp_init(test2);
  token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  printf("Eval = %d (%X)\n", eval_const_int(type_cxt, token), eval_const_int(type_cxt, token));
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "0x2345AbCd";
  cxt = parse_exp_init(test3);
  token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  printf("Eval = %d (%X)\n", eval_const_int(type_cxt, token), eval_const_int(type_cxt, token));
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  type_sys_free(type_cxt);
  printf("Pass!\n");
}

void test_eval_const_token_errors() {
  printf("=== Test eval_const_(type)_token errors() ===\n");
  parse_exp_cxt_t *cxt;
  error_testmode(1);
  int err;  

  err = 0;
  cxt = parse_exp_init("  \'\\xabc\' ");
  if(error_trycatch()) eval_const_char_token(parse_exp(cxt, PARSE_EXP_ALLOWALL));
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);

  err = 0;
  cxt = parse_exp_init("  \'\\5679\' ");
  if(error_trycatch()) eval_const_char_token(parse_exp(cxt, PARSE_EXP_ALLOWALL));
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);

  
  err = 0;
  cxt = parse_exp_init("  \'\\5672\' ");
  if(error_trycatch()) eval_const_char_token(parse_exp(cxt, PARSE_EXP_ALLOWALL));
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);

  err = 0;
  cxt = parse_exp_init("  \'\\m\' ");
  if(error_trycatch()) eval_const_char_token(parse_exp(cxt, PARSE_EXP_ALLOWALL));
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);

  err = 0;
  cxt = parse_exp_init("  \'\\abc\' ");
  if(error_trycatch()) eval_const_char_token(parse_exp(cxt, PARSE_EXP_ALLOWALL));
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);

  err = 0;
  cxt = parse_exp_init("  \'\' ");
  if(error_trycatch()) eval_const_char_token(parse_exp(cxt, PARSE_EXP_ALLOWALL));
  else err = 1;
  assert(err == 1);
  parse_exp_free(cxt);

  printf("Pass!\n");
  return;
}

void test_eval_int_convert() {
  printf("=== Test eval_int_convert() ===\n");
  decl_prop_t int1, int2, ret;
  for(int1 = BASETYPE_CHAR;int1 <= BASETYPE_ULLONG;int1 += 0x00010000) {
    for(int2 = BASETYPE_CHAR;int2 <= BASETYPE_ULLONG;int2 += 0x00010000) {
      ret = eval_int_convert(int1, int2);
      //printf("%X %X %X\n", int1, int2, ret);
      // Note: token_decl_print cannot occur multiple times in printf
      printf("%s + ", token_decl_print(int1));
      printf("%s -> ", token_decl_print(int2));
      printf("%s\n", token_decl_print(ret));
    }
  }
  printf("Pass!\n");
  return;
}

void test_eval_const_char_token() {
  printf("=== Test eval_const_char_token() ===\n");
  token_t *token;
  parse_exp_cxt_t *cxt;
  char ch;

  cxt = parse_exp_init(" \'\\\\\' ");
  ch = eval_const_char_token(token = parse_exp(cxt, PARSE_EXP_ALLOWALL));
  printf("Value %d\n", (int)ch); assert((int)ch == 92);
  parse_exp_free(cxt);
  ast_free(token);

  cxt = parse_exp_init(" \'\\n' ");
  ch = eval_const_char_token(token = parse_exp(cxt, PARSE_EXP_ALLOWALL));
  printf("Value %d\n", (int)ch); assert((int)ch == 10);
  parse_exp_free(cxt);
  ast_free(token);

  cxt = parse_exp_init(" \'\\xab' ");
  ch = eval_const_char_token(token = parse_exp(cxt, PARSE_EXP_ALLOWALL));
  printf("Value %d\n", (int)ch); assert((int)ch == -85);
  parse_exp_free(cxt);
  ast_free(token);

  cxt = parse_exp_init(" \'\\xb' ");
  ch = eval_const_char_token(token = parse_exp(cxt, PARSE_EXP_ALLOWALL));
  printf("Value %d\n", (int)ch); assert((int)ch == 11);
  parse_exp_free(cxt);
  ast_free(token);

  cxt = parse_exp_init(" \'\\777' ");
  ch = eval_const_char_token(token = parse_exp(cxt, PARSE_EXP_ALLOWALL));
  printf("Value %d\n", (int)ch); assert((int)ch == -1);
  parse_exp_free(cxt);
  ast_free(token);

  cxt = parse_exp_init(" \'\\76' ");
  ch = eval_const_char_token(token = parse_exp(cxt, PARSE_EXP_ALLOWALL));
  printf("Value %d\n", (int)ch); assert((int)ch == 62);
  parse_exp_free(cxt);
  ast_free(token);

  printf("Pass!\n");
  return;
}

void test_type_getcomp() {
  printf("=== Test type_getcomp ===\n");
  parse_exp_cxt_t *parse_cxt;
  type_cxt_t *type_cxt;
  token_t *token;
  type_t *type;
  str_t *s;
  char test1[] = "struct a { int * const b; const long c; volatile unsigned char d; }";
  parse_cxt = parse_exp_init(test1);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test1);
  printf("%s\n", s->s);
  str_free(s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "struct { /* extern */ int : 12, **aa /* : 100 */, size_unknown[10 * 2 + 3]; unsigned long long bb : 20; long; } ";
  parse_cxt = parse_exp_init(test2);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test2);
  printf("%s\n", s->s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("=====================================\n");
  char test3[] = "struct {void *;}";
  parse_cxt = parse_exp_init(test3);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test3);
  printf("%s\n", s->s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("=====================================\n"); // Tests nesting of struct and union
  char test4[] = "struct { struct some_struct { int (*(*a)[10])(int x, ...); } var; /*void x*/ union { void (*b)(void); }; }";
  parse_cxt = parse_exp_init(test4);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test4);
  printf("%s\n", s->s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("=====================================\n"); // Tests whether anonymous struct/union is allowed
  char test5[] = "struct name";
  parse_cxt = parse_exp_init(test5);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test5);
  printf("%s\n", s->s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt); 
  ast_free(token);
  printf("=====================================\n"); // Tests promotion within composite types
  char test6[] = "struct { /*int x;*/ const union { volatile int x, y; long zz; }; /*struct named {};*/ struct { volatile int xy[10]; int *z; }; }";
  parse_cxt = parse_exp_init(test6);
  type_cxt = type_sys_init(); 
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test6);
  printf("%s\n", s->s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("=====================================\n"); // Tests promotion within composite types
  char test7[] = "struct { int a; struct { int b : 7, c : 8, d : 10, e, f : 31; int g : 2; }; int h; int i : 15; long j : 33; }";
  parse_cxt = parse_exp_init(test7);
  type_cxt = type_sys_init(); 
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test7);
  printf("%s\n", s->s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("=====================================\n"); // Tests composite type as base type
  char test8[] = "struct name { struct name *ptr; struct name (*)(void)[10] ptr2; }";
  parse_cxt = parse_exp_init(test8);
  type_cxt = type_sys_init(); 
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test8);
  printf("%s\n", s->s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("Pass!\n");
  return;
}

void test_type_getenum() {
  printf("=== Test type_getenum ===\n");
  parse_exp_cxt_t *parse_cxt;
  type_cxt_t *type_cxt;
  token_t *token;
  type_t *type;
  str_t *s;
  char test1[] = "enum enum_name { a = 1, b, c = 10, }"; // Tests named enum
  parse_cxt = parse_exp_init(test1);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test1);
  printf("%s\n", s->s);
  str_free(s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("=====================================\n");
  char test2[] = "struct stru { enum {a,b,c} x; enum {d,e = c * 10 + (2 << 3), /* a -> name clash with previous enum*/} y; }"; // Tests unnamed enum
  parse_cxt = parse_exp_init(test2);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test2);
  printf("%s\n", s->s);
  str_free(s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  printf("Pass!\n");
  return;
}

void test_type_anomaly() {
  printf("=== Test anomalies ===\n");
  error_testmode(1);
  int err;  
  parse_exp_cxt_t *cxt;
  type_cxt_t *type_cxt;
  type_t *type;
  token_t *token;
  str_t *s;
  char test1[] = " volatile const int array[(12 + 8) / 30 - 1] "; // Negative array size
  err = 0;
  cxt = parse_exp_init(test1);
  type_cxt = type_sys_init();
  if(error_trycatch()) {
    token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
    type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  } else { err = 1; }
  assert(err == 1);
  parse_exp_free(cxt);
  type_sys_free(type_cxt);

  char test2[] = " struct str { int x : (12 + 8) / 30 - 1; }"; // Negative bit field size
  err = 0;
  cxt = parse_exp_init(test2);
  type_cxt = type_sys_init();
  if(error_trycatch()) {
    token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
    type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  } else { err = 1; }
  assert(err == 1);
  parse_exp_free(cxt);
  type_sys_free(type_cxt);

  char test3[] = " struct str { int x : (12UL + 8LL) / 30 - 1; }"; // Negative bit field size
  err = 0;
  cxt = parse_exp_init(test3);
  type_cxt = type_sys_init();
  if(error_trycatch()) {
    token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
    type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  } else { err = 1; }
  assert(err == 1);
  parse_exp_free(cxt);
  type_sys_free(type_cxt);

  char test4[] = " char array['A' + 'b']"; // char constant as array size
  err = 0;
  cxt = parse_exp_init(test4);
  type_cxt = type_sys_init();
  if(error_trycatch()) {
    token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
    type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  } else { err = 1; }
  assert(err == 1);
  parse_exp_free(cxt);
  type_sys_free(type_cxt);

  char test5[] = " char array[(char (*)(void))x]"; // char constant as array size
  err = 0;
  cxt = parse_exp_init(test5);
  type_cxt = type_sys_init();
  if(error_trycatch()) {
    token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
    type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  } else { err = 1; }
  assert(err == 1);
  parse_exp_free(cxt);
  type_sys_free(type_cxt);

  char test6[] = " char array[(unsigned long long)x]"; // char constant as array size
  err = 0;
  cxt = parse_exp_init(test6);
  type_cxt = type_sys_init();
  if(error_trycatch()) {
    token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
    type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  } else { err = 1; }
  assert(err == 1);
  parse_exp_free(cxt);
  type_sys_free(type_cxt);

  char test7[] = " char array[sizeof(int *[])]"; // char constant as array size
  err = 0;
  cxt = parse_exp_init(test7);
  type_cxt = type_sys_init();
  if(error_trycatch()) {
    token = parse_decl(cxt, PARSE_DECL_HASBASETYPE);
    type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
    //ast_print(token, 0); // Print after this to get array size info
  } else { err = 1; }
  assert(err == 1);
  parse_exp_free(cxt);
  type_sys_free(type_cxt);
  printf("Pass!\n");
  (void)type;
  return;
}

void test_eval_const_str_token() {
  printf("=== Test eval_const_str_token() ===\n");
  token_t *token;
  parse_exp_cxt_t *cxt;
  str_t *s = NULL, *printable = NULL;

  cxt = parse_exp_init(" \"abcdefg\\n\\r\\v\\b\\t \\\" \\\' \\\\ \\xff \\765\" ");
  s = eval_const_str_token(token = parse_exp(cxt, PARSE_EXP_ALLOWALL));
  printable = eval_print_const_str(s);
  printf("str = -->%s<--\n", str_cstr(printable));
  parse_exp_free(cxt);
  ast_free(token);
  if(s) str_free(s);
  if(printable) str_free(printable);

  printf("Pass!\n");
  return;
}

void test_type_typeof() {
  printf("=== Test type_typeof ===\n");
  parse_exp_cxt_t *parse_cxt;
  type_cxt_t *type_cxt;
  token_t *token;
  type_t *type;
  str_t *s;
  
  // First test whether get string type works
  type_cxt = type_sys_init();
  type = type_get_strliteral(type_cxt, 25);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", s->s);
  str_free(s);
  type_sys_free(type_cxt);
  printf("=====================================\n");
  /*
  char test1[] = "enum enum_name { a = 1, b, c = 10, }"; // Tests named enum
  parse_cxt = parse_exp_init(test1);
  type_cxt = type_sys_init();
  token = parse_decl(parse_cxt, PARSE_DECL_HASBASETYPE);
  assert(token_get_next(parse_cxt->token_cxt) == NULL);
  ast_print(token, 0);
  type = type_gettype(type_cxt, token, ast_getchild(token, 0), 0);
  s = type_print(type, NULL, NULL, 1, 0);
  printf("%s\n", test1);
  printf("%s\n", s->s);
  str_free(s);
  type_sys_free(type_cxt);
  parse_exp_free(parse_cxt);
  ast_free(token);
  */

  printf("Pass!\n");
  return;
}

int main() {
  printf("=== Hello World! ===\n");
  test_scope_init();
  test_bintree();
  test_list();
  test_str();
  test_vector();
  test_eval_const_char_token();
  test_eval_const_token_errors(); // Memory leak
  test_eval_int_convert();
  test_eval_const_int();
  test_type_getcomp();
  test_type_getenum();
  test_type_anomaly();
  test_eval_const_str_token();
  test_type_typeof();
  return 0;
}
  
