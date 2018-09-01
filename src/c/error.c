
#include "error.h"

// This global pointer holds the begin of the text. We use this pointer and 
// a given pointer to compute the line and column number
static const char *begin = NULL;

void error_init(const char *s) { begin = s; }
void error_free() {}

// Returns the row and column of a given pointer
void error_get_row_col(const char *s, int *row, int *col) {
  *row = *col = 1;
  for(const char *p = begin; p != s && *p != '\0';p++) {
    if(*p == '\n') (*row)++, *col = 1;
    else (*col)++;
  return;
}