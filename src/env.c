
#include "env.h"

// This function initializes inclusion path from multiple sources
void env_init_include_path(env_t *env) {
  int count = 0;
  // Read environmental variable; These paths are inserted into the beginning of the list, i.e.,
  // they will override all other paths
  char *env_path = getenv("C_INCLUDE_PATH");
  if(env_path != NULL) {

    count++;
  }
  return;
}

env_t *env_init() {
  env_t *env = (env_t *)malloc(sizeof(env_t));
  SYSEXPECT(env != NULL);
  memset(env, 0x00, sizeof(env_t));
  env->include_paths = list_init();
  return env;
}

void env_free(env_t *env) {
  do {
    listnode_t *node = list_head(env->include_paths);
    while(node != NULL) {
      free(node->key);
      node = list_next(node);
    }
    list_free(env->include_paths);
  } while(0);
  free(env);
  return;
}
