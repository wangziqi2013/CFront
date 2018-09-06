
// Note that this might be a common name, so we make it longer to avoid conflict
#ifndef _ERROR_H_CFRONT
#define _ERROR_H_CFRONT

#include <stdio.h>

#define error_exit(fmt, ...) do { fprintf(stderr, "Error: " fmt, ##__VA_ARGS__); exit(1); } while(0);
#define error_row_col_exit(s, fmt, ...) do { \
                                          int row, col; error_get_row_col(s, &row, &col); \
                                          fprintf(stderr, "Error (row %d col %d): " fmt, row, col, ##__VA_ARGS__); \
                                          exit(1); } while(0);

void error_init(const char *s);
void error_free();
void error_testmode(int mode);
void error_get_row_col(const char *s, int *row, int *col);

#endif

