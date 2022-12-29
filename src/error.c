
#include "error.h"

// This global pointer holds the begin of the text. We use this pointer and 
// a given pointer to compute the line and column number
static const char *begin = NULL;
static int inited = 0;
// Whether test mode is on. Under test mode, error reporting functions calls 
// longjmp to jump to a previously set location
static int testmode = 0;
jmp_buf env;

// This must be called in order for line number to work
void error_init(const char *s) { begin = s; inited = 1; }
void error_free() { inited = 0; }
void error_testmode(int mode) { testmode = mode; }
void error_exit_or_jump(int need_exit) { 
  if(testmode) { fprintf(stderr, "*** %s are redirected ***\n", need_exit ? "Errors" : "Warnings"); longjmp(env, 1); }
#ifndef NDEBUG
  else if(need_exit) { assert(0); }
#else
  else if(need_exit) { exit(ERROR_CODE_EXIT); }
#endif
}

// Returns the row and column of a given pointer
// Note:
//   1. If error is not initialized then row and col will be set to -1
//   2. If the pointer is not in the string registered during initialization
//      then row and col will be set to -2
void error_get_row_col(const char *s, int *row, int *col) {
  if(inited == 0) { *row = *col = -1; }
  else {
    *row = *col = 1;
    const char *p;
    const char *line_head = begin; // Track the beginning of the line
    for(p = begin; p != s && *p != '\0';p++) {
      if(*p == '\n') (*row)++, *col = 1, line_head = p + 1;
      else (*col)++;
    }
    if(*p == '\0' && p != s) { // if p == s then still valid
      *row = *col = -2;
      fprintf(stderr, "Did you forget to register a new pointer with error module?\n");
    } else { // Print from line head to next line
      printf("----\n");
      while(*line_head != '\n' && *line_head != '\0') putchar(*line_head++);
      putchar('\n');
      for(int i = 0;i < *col - 1;i++) putchar(' ');
      printf("^\n");
      printf("----\n");
    }
  }
  return;
}

void syserror(const char *prompt) { 
  fputs(prompt, stderr);
  exit(ERROR_CODE_EXIT); 
}

int error_get_offset(const char *offset) { 
  return offset - begin + 1; // Begin with column 1 
} 
