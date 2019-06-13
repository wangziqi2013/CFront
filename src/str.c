
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "error.h"

str_t *str_init() {
  str_t *str = (str_t *)malloc(sizeof(str_t));
  SYSEXPECT(str != NULL);
  str->s = (char *)malloc(STR_INIT_SIZE + 1);
  SYSEXPECT(str->s != NULL);
  str->capacity = STR_INIT_SIZE;
  str->size = 0;
  str->s[0] = '\0';
  return str;
}
void str_free(str_t *str) { free(str->s); free(str); }
void str_clear(str_t *str) { str_s[0] = '\0'; str->size = 0; }
int str_size(str_t *str) { return str->size; }

// Realloc the buffer to hold at least size + 1 bytes
void str_extend(str_t *str, int size) {
  if(size > str->capacity) {
    str->s = realloc(str->s, size + 1);
    SYSEXPECT(str->s != NULL);
    str->capacity = size;
  }
}

void str_append(str_t *str, char ch) {
  if(str->size == str->capacity) str_extend(str, str->capacity * 2);
  assert(str->size < str->capacity);
  str->s[str->size++] = ch;
  str->s[str->size] = '\0';
}

void str_prepend(str_t *str, char ch) {
  if(str->size == str->capacity) str_extend(str, str->capacity * 2);
  assert(str->size < str->capacity);
  memmove(str->s + 1, str->s, str->size + 1); // Including the trailing zero
  str->s[0] = ch;
  str->size++;
}

void str_prepend_str(str_t *str, const char *s) {
  int copylen = strlen(s);
  if(str->size + copylen >= str->capacity) str_extend(str, str->size + copylen);
  assert(str->size + copylen <= str->capacity);
  memmove(str->s + copylen, str->s, str->size + 1); // Including the trailing zero
  memcpy(str->s, s, copylen); // Do not include the trailing zero
  str->size += copylen;
}

void str_concat(str_t *str, const char *s) {
  int copylen = strlen(s);
  if(str->size + copylen >= str->capacity) str_extend(str, str->size + copylen);
  assert(str->size + copylen <= str->capacity);
  memmove(str->s + str->size, s, copylen + 1); // Includes the '\0'
  str->size += copylen;
}

void str_print_int(str_t *str, int d) {
  char temp[MAX_INT_DIGITS];
  sprintf(temp, "%d", d);
  str_concat(str, temp);
}

char *str_copy(const str_t *str) { // Returns a string allocated from heap. The str is not changed
  char *s = (char *)malloc(str->size + 1);
  SYSEXPECT(s != NULL);
  memcpy(s, str->s, str->size + 1);
  return s;
}

vector_t *vector_init() {
  vector_t *vector = (vector_t *)malloc(sizeof(vector_t));
  SYSEXPECT(vector != NULL);
  vector->data = (void **)malloc(VECTOR_INIT_SIZE * sizeof(void *));
  SYSEXPECT(vector->data != NULL);
  vector->size = 0;
  vector->capacity = VECTOR_INIT_SIZE;
  return vector;
}
void vector_free(vector_t *vector) { free(vector->data); free(vector); }
int vector_size(vector_t *vector) { return vector->size; }

void vector_extend(vector_t *vector, int size) {
  if(size > vector->capacity) {
    vector->capacity = size;
    vector->data = realloc(vector->data, size * sizeof(void *));
    SYSEXPECT(vector->data != NULL);
  }
  return;
}

void vector_append(vector_t *vector, void *value) {
  if(vector->size == vector->capacity) vector_extend(vector, vector->size * 2);
  assert(vector->size < vector->capacity);
  vector->data[vector->size++] = value;
  return;
}

void *vector_at(vector_t *vector, int index) {
  assert(index < vector->size && index >= 0);
  return vector->data[index];
}

void **vector_addrat(vector_t *vector, int index) {
  assert(index < vector->capacity && index >= 0);  // Since we only take address, use capacity here
  return vector->data + index;
}