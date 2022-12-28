
// This file implements global environmental variables

#ifndef _CFRONT_ENV_H
#define _CFRONT_ENV_H

#include "hashtable.h"
#include "list.h"

typedef struct {
  // Search path for included files
  list_t *include_paths;
} env_t;

void env_init_include_path(env_t *env);

env_t *env_init();
void env_free(env_t *env);

#endif