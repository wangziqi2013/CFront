
#include <stdio.h>
#include <stdlib.h>
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
  if(str->size == str->capacity) str_extend(str, str->size * 2);
  str->s[str->size++] = ch;
  str->s[str->size] = '\0';
}
void str_concat(str_t *str, const char *s) { while(*s) str_append(*s++); }

char *str_copy(const str_t *str) { // Returns a string allocated from heap. The str is not changed
  char *s = (char *)malloc(str->size + 1);
  SYSEXPECT(s != NULL);
  memcpy(s, str->s, str->size + 1);
  return s;
}