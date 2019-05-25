
#ifndef _STR_H
#define _STR_H

#define STR_INIT_SIZE 32     // Excluding the terminating 0
#define VECTOR_INIT_SIZE 32
#define MAX_INT_DIGITS 64    // Can't be that long...

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
void str_prepend(str_t *str, char ch);
void str_concat(str_t *str, const char *s);
void str_prepend_str(str_t *str, const char *s);
void str_print_int(str_t *str, int d);  // Append an integer at the end of the str
char *str_copy(const str_t *str);
static inline char *str_cstr(str_t *s) { return s->s; }

typedef struct {
  int size, capacity;
  void **data;
} vector_t;

vector_t *vector_init();
void vector_free(vector_t *vector);
int vector_size(vector_t *vector);
void vector_extend(vector_t *vector, int size);
void vector_append(vector_t *vector, void *value);
void *vector_at(vector_t *vector, int index);
void **vector_addrat(vector_t *vector, int index);

#endif