
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

void test_scope_init() {
  printf("=== Test Scope Init ===\n");
  type_cxt_t *cxt = type_init();
  scope_top_insert(cxt, SCOPE_STRUCT, "wangziqi2013", (void *)0x12345UL);
  scope_top_insert(cxt, SCOPE_UNION, "wangziqi2016", (void *)0x23456UL);
  assert(scope_search(cxt, SCOPE_STRUCT, "wangziqi2013") == (void *)0x12345UL);
  assert(scope_search(cxt, SCOPE_UNION, "wangziqi2016") == (void *)0x23456UL);
  scope_recurse(cxt); // 2 levels
  assert(scope_top_find(cxt, SCOPE_STRUCT, "wangziqi2013") == HT_NOTFOUND);
  assert(scope_top_find(cxt, SCOPE_UNION, "wangziqi2016") == HT_NOTFOUND);
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
  type_free(cxt);
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

    list_t *list = list_str_init();
    for(int i = 0;i < test_size;i++) {
      results[i] = list_insert_nodup(list, tests[i], tests[i]);
    }
    
    for(int i = test_size - 1;i >= 0;i--) {
      void *ret = list_find(list, tests[i]);
      assert(ret != HT_NOTFOUND);
      assert(strcmp(ret, tests[i]) == 0);
    }

    assert(list_find(list, "wangziqi2013") == HT_NOTFOUND);
    assert(list_find(list, "+_1234567890") == HT_NOTFOUND);
    assert(list_find(list, "!@#$") == HT_NOTFOUND);
    assert(list_find(list, "QWERT[]{}") == HT_NOTFOUND);
    printf("Finished: %06d [ Size: %d ]\r", seed, list_size(list));
    //assert(list_size(list) == test_size);
    int mid1 = test_size / 4;
    int mid2 = mid1 * 2;
    int mid3 = mid1 * 3;
    for(int i = mid1;i < mid2;i++) {  // Ascending removal from middle
      assert(list_remove(list, tests[i]) == tests[i]);
      assert(list_find(list, tests[i]) == BT_NOTFOUND);
      assert(list_remove(list, tests[i]) == BT_NOTFOUND);
    }
    for(int i = mid3 - 1;i >= mid2;i--) {  // Decending removal from middle
      assert(list_remove(list, tests[i]) == tests[i]);
      assert(list_find(list, tests[i]) == BT_NOTFOUND);
      assert(list_remove(list, tests[i]) == BT_NOTFOUND);
    }
    for(int i = 0;i < mid1;i++) { // Remove from beginning
      assert(list_remove(list, tests[i]) == tests[i]);
      assert(list_find(list, tests[i]) == BT_NOTFOUND);
      assert(list_remove(list, tests[i]) == BT_NOTFOUND);
    }
    for(int i = test_size - 1;i >= mid3;i--) { // Remove from end
      assert(list_remove(list, tests[i]) == tests[i]);
      assert(list_find(list, tests[i]) == BT_NOTFOUND);
      assert(list_remove(list, tests[i]) == BT_NOTFOUND);
    }
    assert(list_size(list) == 0);
    assert(list->head == list->tail && list->head == NULL);

    for(int i = 0;i < test_size;i++) free(tests[i]);
    free(tests);
    free(results);
    list_free(list);
  }
  putchar('\n');
  list_t *list = list_str_init();
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

void test_eval_const() {
  printf("=== Test eval_const() ===\n");
  char test1[] = "1 + 2 * 3 + 6 / 2";
  cxt = parse_exp_init(test1);
  token = parse_exp(cxt, PARSE_EXP_ALLOWALL);
  assert(token_get_next(cxt->token_cxt) == NULL);
  ast_print(token, 0);
  printf("Eval = %d\n", eval_const(token));
  parse_exp_free(cxt);
  ast_free(token);
  printf("=====================================\n");
  printf("Pass!\n");
}

void test_getimm() {
  printf("=== Test eval_get[type]imm() ===\n");

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
  test_getimm();
  test_eval_const();
  return 0;
}
  
