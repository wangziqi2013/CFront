
// This file implements global environmental variables

#ifndef _CFRONT_ENV_H
#define _CFRONT_ENV_H

#include "hashtable.h"

typedef struct {
  // Search path for included files
  list_t *include_paths;
} env_t;

env_t *env_init();
void env_free(env_t *env);

#endif