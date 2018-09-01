
#include "error.h"

// This global pointer holds the begin of the text. We use this pointer and 
// a given pointer to compute the line and column number
static const char *begin = NULL;
static int inited = 0;

// This must be called in order for line number to work
void error_init(const char *s) { begin = s; inited = 1; }
void error_free() { inited = 0; }

// Returns the row and column of a given pointer
// Note:
//   1. If error is not initialized then row and col will be set to -1
//   2. If the pointer is not in the string registerd during initialization
//      then row and col will be set to -2
void error_get_row_col(const char *s, int *row, int *col) {
  if(inited == 0) {
    *row = *col = -1;
  } else {
    *row = *col = 1;
    const char *p;
    for(p = begin; p != s && *p != '\0';p++) {
      if(*p == '\n') (*row)++, *col = 1;
      else (*col)++;
    }
    if(*p == '\0') *row = *col = -2;
  }
  return;
}