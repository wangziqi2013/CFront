
#include <stdio.h>
#include <stdlib.h>
#include "str.h"

str_t *str_init() {
  str_t *str = (str_t *)malloc(sizeof(str_t));
  SYSEXPECT(str != NULL);
  str->s = (char *)malloc(STR_INIT_SIZE);
  SYSEXPECT(str->s != NULL);
  str->capacity = STR_INIT_SIZE;
  str->size = 0;
  return str;
}

void str_free(str_t *str) { free(str->s); free(str); }