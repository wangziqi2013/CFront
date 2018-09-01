
// Note that this might be a common name, so we make it longer to avoid conflict
#ifndef _ERROR_H_CFRONT
#define _ERROR_H_CFRONT

#include <stdio.h>

#define error_exit(fmt, ...) do { fprintf(stderr, "Error: " fmt, ##__VA_ARGS__); exit(1); } while(0);

#endif

