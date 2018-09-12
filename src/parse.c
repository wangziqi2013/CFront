
#include "parse.h"

parse_stmt_cxt_t *parse_init(char *input) { return parse_exp_init(input); }
void parse_free(parse_cxt_t *cxt) { parse_exp_free(cxt); }

// Five cases:
//  1. Base type + ';' must be a type declaration, most likely struct/union/enum
//  2. Base type + decl + "," must be a type declaration or data definition
//  3. Base type + decl + "=" must be a data definition with initializer
//  4. Base type + decl + ";" must be a type declatation
//  5. Base type + func decl + '{' must be function definition
token_t *parse(parse_cxt_t *cxt) {
  token_t *root = token_alloc_type(T_ROOT);
  while(1) {
    if(token_lookahead(cxt->token_cxt, 1) == NULL) break; // Reached EOF
    token_t *basetype = parse_decl_basetype(cxt);
    if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_SEMICOLON) { // Case 1
      token_consume_type(cxt->token_cxt, T_SEMICOLON);
      ast_append_child(root, ast_append_child(token_alloc_type(T_GLOBAL_DECL_ENTRY), basetype));
      continue;
    }
    token_t *decl = parse_decl(cxt, PARSE_DECL_NOBASETYPE);
    token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
    if(la->type == T_LCPAREN) { // Case 5
      assert(ast_getchild(decl, 0) != NULL);
      if(ast_getchild(decl, 0)->type != EXP_FUNC_CALL) // Only function type could have a body
        error_row_col_exit(cxt->token_cxt->s, "Only function definition can have a body\n");
      token_t *comp_stmt = parse_comp_stmt(cxt);
      ast_push_child(decl, basetype);
      ast_append_child(root, ast_append_child(ast_append_child(token_alloc_type(T_GLOBAL_FUNC), decl), comp_stmt));
      continue;
    }
    token_t *entry = ast_append_child(token_alloc_type(T_GLOBAL_DECL_ENTRY), basetype);
    ast_append_child(root, entry);
    while(1) {
      token_t *var = ast_append_child(token_alloc_type(T_GLOBAL_DECL_VAR), decl);
      ast_append_child(entry, var);
      if(la->type == T_ASSIGN) {
        token_consume_type(cxt->token_cxt, T_ASSIGN);
        if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_LCPAREN) ast_append_child(var, parse_init_list(cxt));
        else ast_append_child(var, parse_exp(cxt, PARSE_EXP_NOCOMMA));
        la = token_lookahead_notnull(cxt->token_cxt, 1);
      }
      if(la->type == T_SEMICOLON) { token_consume_type(cxt->token_cxt, T_SEMICOLON); break; }
      else if(la->type == T_COMMA) {
        token_consume_type(cxt->token_cxt, T_COMMA);
        decl = parse_decl(cxt, PARSE_DECL_NOBASETYPE);
        continue;
      } else error_row_col_exit(la->offset, "Expecting \',\', \'=\' or \';\' for global declaration\n");
    }
  }
  return root;
}