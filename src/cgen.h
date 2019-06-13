
#ifndef _CGEN_H
#define _CGEN_H

#include "type.h"
#include "ast.h"

void cgen_global_decl(type_cxt_t *cxt, token_t *global_decl);
void cgen_global_func(type_cxt_t *cxt, token_t *func);
void cgen(type_cxt_t *cxt, token_t *root);

#endif