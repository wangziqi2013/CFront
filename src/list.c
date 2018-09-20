
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "list.h"

list_t *list_init() {
  list_t *list = (list_t *)malloc(sizeof(list_t));
  if(list == NULL) syserror(__func__);
  list->size = 0;
  list->head = NULL;
  return list;
}