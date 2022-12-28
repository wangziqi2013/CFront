
#include "env.h"

env_t *env_init() {
  env_t *env = (env_t *)malloc(sizeof(env_t));
  SYSEXPECT(env != NULL);
  memset(env, 0x00, sizeof(env_t));
  env->include_paths = list_init();
  return;
}

void env_free(env_t *env) {
  list_free(env->include_paths);
  free(env);
  return;
}
