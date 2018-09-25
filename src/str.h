
#ifndef _STR_H
#define _STR_H

#define STR_INIT_SIZE 32

typedef struct {
  int size;
  int capacity;
  char *s;
} str_t;

str_t *str_init();
void str_free(str_t *str);

#endif