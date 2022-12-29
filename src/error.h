
// Note that this might be a common name, so we make it longer to avoid conflict
#ifndef _ERROR_H_CFRONT
#define _ERROR_H_CFRONT

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>

extern jmp_buf env;

#define ERROR_CODE_EXIT 1
// Input to function error_exit_or_jump()
#define ERROR_ACTION_CONT 0
#define ERROR_ACTION_EXIT 1
#define error_exit(fmt, ...) do { fprintf(stderr, "Error: " fmt, ##__VA_ARGS__); error_exit_or_jump(ERROR_ACTION_EXIT); } while(0);
#define error_row_col_exit(s, fmt, ...) do { \
                                          int row, col; error_get_row_col(s, &row, &col); \
                                          fprintf(stderr, "Error (row %d col %d): " fmt, row, col, ##__VA_ARGS__); \
                                          error_exit_or_jump(ERROR_ACTION_EXIT); } while(0);
#define warn_row_col_exit(s, fmt, ...) do { \
                                          int row, col; error_get_row_col(s, &row, &col); \
                                          fprintf(stderr, "Warning (row %d col %d): " fmt, row, col, ##__VA_ARGS__); \
                                          error_exit_or_jump(ERROR_ACTION_CONT); } while(0);

// The following two macros are used for testing. It redirects the control flow back to the testing function
// if an error occurs. The testing function should set testmode to 1.
// Usage: if(error_trycatch()) { ...code goes here } else { ... error happens } ... error did not happen
#define error_trycatch() (setjmp(env) == ERROR_FIRSTTIME)
#define ERROR_FIRSTTIME 0

#define SYSEXPECT(expr) do { if(!(expr)) syserror(__func__); } while(0) // Assertion for system calls; Valid under all modes

void error_init(const char *s);
void error_free();
void error_testmode(int mode);
void error_exit_or_jump(int need_exit);
void error_get_row_col(const char *s, int *row, int *col);
void syserror(const char *prompt);

int error_get_offset(const char *offset); // Returns integer offset

#endif

