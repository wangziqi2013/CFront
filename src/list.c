
#include "list.h"

list_t *list_init(eq_cb_t eq) {
  list_t *list = (list_t *)malloc(sizeof(list_t));
  SYSEXPECT(list != NULL);
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

int list_size(list_t *list) { return list->size; }

// Allocate a node. All fields are uninitialized
listnode_t *listnode_alloc() {
  listnode_t *node = (listnode_t *)malloc(sizeof(listnode_t));
  SYSEXPECT(node != NULL);
  return node;
}
void listnode_free(listnode_t *node) { free(node); }

// Always insert to the end of the list; do not check for duplicate; Always return the inserted value
void *list_insert(list_t *list, void *key, void *value) {
  listnode_t *node = listnode_alloc();
  node->key = key;
  node->value = value;
  node->next = NULL;
  assert(list->head || !list->tail);  // If head is NULL then tail must also be NULL
  if(list->head == NULL) list->head = list->tail = node;
  else list->tail = (list->tail->next = node);
  list->size++;
  return value;
}

// Inserts before the node specified by index; if index == list size then insert at the end
listnode_t *list_insertat(list_t *list, void *key, void *value, int index) {
  assert(index <= list->size && index >= 0);
  if(index == list->size) return list_insert(list, key, value); // Empty insert will be caught here
  assert(list->size > 0);
  list->size++;
  listnode_t *node = listnode_alloc();
  node->key = key;
  node->value = value;
  if(index == 0) {
    node->next = list->head;
    list->head = node;
    assert(list->tail);
  } else {
    listnode_t *curr = list->head;
    while(--index != 0) curr = curr->next;
    node->next = curr->next;
    curr->next = node;
    assert(curr->next);
  }
  return value;
}

void *list_insert_nodup(list_t *list, void *key, void *value) {
  void *ret = list_find(list, key);
  if(ret == LIST_NOTFOUND) value = list_insert(list, key, value);
  return ret;
}

// Search for the given key, and return value; Return LIST_NOTFOUND if not found
void *list_find(list_t *list, void *key) {
  listnode_t *curr = list->head;
  while(curr) if(list->eq(key, curr->key)) return curr->value; else curr = curr->next;
  return LIST_NOTFOUND;
}

// Returns the node specified by the index; If index is too large then return LIST_NOTFOUND. 
// Index must be positive
const listnode_t *list_findat(list_t *list, int index) {
  assert(index >= 0);
  if(index >= list->size) return LIST_NOTFOUND;
  listnode_t *curr = list->head;
  while(index--) curr = curr->next;
  return curr;
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
    if(curr && list->eq(curr->key, key)) {
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

// Value is returned, and the second argument holds the key
void *list_removeat(list_t *list, int index, void **key) {
  assert(index >= 0);
  if(index >= list->size) return LIST_NOTFOUND;
  listnode_t *curr = list->head, *prev = curr;
  void *ret = NULL;
  if(index == 0) {
    list->head = curr->next;  // Could be NULL
    ret = curr->value;
    *key = curr->key;
    listnode_free(curr);
    list->size--;
    if(curr == list->tail) list->tail = NULL;
    return ret;
  }
  while(index--) {
    curr = curr->next;
    prev = curr;
  }
  prev->next = curr->next;
  ret = curr->value;
  *key = curr->key;
  listnode_free(curr);
  list->size--;
  if(curr == list->tail) list->tail = prev; // If deleting the last element then adjust tail
  return ret;
}