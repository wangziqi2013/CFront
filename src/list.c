
#include "list.h"

list_t *list_init(eq_cb_t eq) {
  list_t *list = (list_t *)malloc(sizeof(list_t));
  if(list == NULL) syserror(__func__);
  list->size = 0;
  list->head = list->tail = NULL;
  list->eq = eq;
  return list;
}
list_t *list_str_init() { return list_init(streq_cb); }

void list_free(list_t *list) {
  assert(list->head || !list->tail);
  listnode_t *node = list->head, *prev = node;
  if(node) do {
    node = node->next;
    listnode_free(prev);
    prev = node;
  } while(node);
  free(list);
  return;
}

// Allocate a node. All fields are uninitialized
listnode_t *listnode_alloc() {
  listnode_t *node = (listnode_t *)malloc(sizeof(listnode_t));
  if(node == NULL) syserror(__func__);
  return node;
}
void listnode_free(listnode_t *node) { free(node); }

// Always insert to the end of the list; do not check for duplicate
void list_insert(list_t *list, void *key, void *value) {
  listnode_t *node = listnode_alloc();
  node->key = key;
  node->value = value;
  node->next = NULL;
  assert(list->head || !list->tail);  // If head is NULL then tail must also be NULL
  if(list->head == NULL) list->head = list->tail = node;
  else list->tail = (list->tail->next = node);
  list->size++;
  return;
}

// Search for the given key, and return value; Return LIST_NOTFOUND if not found
void *list_find(list_t *list, void *key) {
  listnode_t *curr = list->head;
  while(curr) if(list->eq(key, curr->key)) return curr->value;
  return LIST_NOTFOUND;
}

// Removes the key from the list. Return value if key exists; LIST_NOTFOUND otherwise
void *list_remove(list_t *list, void *key) {
  listnode_t *curr = list->head, *prev = curr;
  if(curr == NULL) return LIST_NOTFOUND;
  void *ret = NULL;
  if(list->eq(curr->key, key)) {
    list->head = curr->next;  // Could be NULL
    ret = curr->value;
    listnode_free(curr);
    list->size--;
    if(curr == list->tail) list->tail = NULL;
    return ret;
  }
  do {
    curr = curr->next;
    if(list->eq(curr->key, key)) {
      prev->next = curr->next;
      ret = curr->value;
      listnode_free(curr);
      list->size--;
      if(curr == list->tail) list->tail = prev; // If deleting the last element then adjust tail
      return ret;
    }
    prev = curr;
  } while(curr);
  return LIST_NOTFOUND;
}