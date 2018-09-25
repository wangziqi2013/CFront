
#ifndef _STR_H
#define _STR_H

#define STR_INIT_SIZE 32 // Excluding the terminating 0

typedef struct {
  int size;
  int capacity;          // Both excluding the terminating 0
  char *s;
} str_t;

str_t *str_init();
void str_free(str_t *str);
int str_size(str_t *str);
void str_extend(str_t *str, int size);
void str_append(str_t *str, char ch);
void str_concat(str_t *str, const char *s);

#endif