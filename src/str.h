
#ifndef _STR_H
#define _STR_H

#define STR_INIT_SIZE 32     // Excluding the terminating 0
#define VECTOR_INIT_SIZE 32

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
char *str_copy(const str_t *str);

typedef struct {
  int size, capacity;
  void **data;
} vector_t;

vector_t *vector_init();
void vector_free(vector_t *vector);
void vector_extend(vector_t *vector, int size);

#endif