
#include "parse_stmt.h"
#include "parse_decl.h"

parse_stmt_cxt_t *parse_stmt_init(char *input) { return parse_exp_init(input); }
void parse_stmt_free(parse_stmt_cxt_t *cxt) { parse_exp_free(cxt); }

// Return a labeled statement
token_t *parse_lbl_stmt(parse_stmt_cxt_t *cxt, token_type_t type) {
  if(type == T_IDENT) {
    token_t *token = token_alloc_type(T_LBL_STMT);
    ast_append_child(token, token_get_next(cxt->token_cxt));
    if(!token_consume_type(cxt->token_cxt, T_COLON)) assert(0); // Caller guarantees this
    return ast_append_child(token, parse_stmt(cxt));
  }  
  token_t *token = token_get_next(cxt->token_cxt);
  if(type == T_CASE) ast_append_child(token, parse_exp(cxt, PARSE_EXP_NOCOLON));
  if(!token_consume_type(cxt->token_cxt, T_COLON))
    error_row_col_exit(token->offset, "Expecting \':\' for \"%s\" statement\n", token_symstr(token->type));
  return ast_append_child(token, parse_stmt(cxt));
}

// Returns an expression statement
token_t *parse_exp_stmt(parse_stmt_cxt_t *cxt) {
  token_t *token = ast_append_child(token_alloc_type(T_EXP_STMT), parse_exp(cxt, PARSE_EXP_ALLOWALL));
  if(!token_consume_type(cxt->token_cxt, T_SEMICOLON))
    error_row_col_exit(cxt->token_cxt->s, "Expecting \';\' after expression statement\n");
  return token;
}

token_t *parse_comp_stmt(parse_stmt_cxt_t *cxt) {
  token_t *decl_list = token_alloc_type(T_DECL_STMT_LIST);
  token_t *stmt_list = token_alloc_type(T_STMT_LIST);
  token_t *root = ast_append_child(ast_append_child(token_alloc_type(T_COMP_STMT), decl_list), stmt_list);
  assert(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_LCPAREN);
  token_consume_type(cxt->token_cxt, T_LCPAREN);
  while(parse_decl_isbasetype(cxt, token_lookahead_notnull(cxt->token_cxt, 1))) { // Loop through lines
    token_t *decl_entry = ast_append_child(token_alloc_type(T_DECL_STMT_ENTRY), parse_decl_basetype(cxt));
    ast_append_child(decl_list, decl_entry);
    while(1) { // Loop through variables
      token_t *var = ast_append_child(token_alloc_type(T_DECL_STMT_VAR), parse_decl(cxt, PARSE_DECL_NOBASETYPE));
      ast_append_child(decl_entry, var);
      token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
      if(la->type == T_ASSIGN) {
        token_consume_type(cxt->token_cxt, T_ASSIGN);
        if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_LCPAREN) ast_append_child(var, parse_init_list(cxt));
        else ast_append_child(var, parse_exp(cxt, PARSE_EXP_NOCOMMA));
        la = token_lookahead_notnull(cxt->token_cxt, 1);
      }
      if(la->type == T_COMMA) { token_consume_type(cxt->token_cxt, T_COMMA); continue; }
      else if(la->type == T_SEMICOLON) { token_consume_type(cxt->token_cxt, T_SEMICOLON); break; }
      else { error_row_col_exit(la->offset, "Expecting \',\' or \';\' after variable declaration\n"); }
    }
  } // Then parse statement list
  while(token_lookahead_notnull(cxt->token_cxt, 1)->type != T_RCPAREN) ast_append_child(stmt_list, parse_stmt(cxt));
  token_consume_type(cxt->token_cxt, T_RCPAREN);
  return root;
}

token_t *parse_if_stmt(parse_stmt_cxt_t *cxt) {
  assert(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_IF);
  token_t *if_stmt = token_get_next(cxt->token_cxt);
  if(!token_consume_type(cxt->token_cxt, T_LPAREN)) error_row_col_exit(if_stmt->offset, "Expecting \'(\' after \"if\"\n");
  ast_append_child(if_stmt, parse_exp(cxt, PARSE_EXP_ALLOWALL));
  if(!token_consume_type(cxt->token_cxt, T_RPAREN)) error_row_col_exit(if_stmt->offset, "Expecting \')\' after \"if\" expression\n");
  ast_append_child(if_stmt, parse_stmt(cxt));
  if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_ELSE) {
    token_t *else_stmt = token_get_next(cxt->token_cxt);
    ast_append_child(if_stmt, else_stmt);
    ast_append_child(else_stmt, parse_stmt(cxt));
  }
  return if_stmt;
}

token_t *parse_switch_stmt(parse_stmt_cxt_t *cxt) {
  assert(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_SWITCH);
  token_t *switch_stmt = token_get_next(cxt->token_cxt);
  if(!token_consume_type(cxt->token_cxt, T_LPAREN)) error_row_col_exit(switch_stmt->offset, "Expecting \'(\' after \"switch\"\n");
  ast_append_child(switch_stmt, parse_exp(cxt, PARSE_EXP_ALLOWALL));
  if(!token_consume_type(cxt->token_cxt, T_RPAREN)) error_row_col_exit(switch_stmt->offset, "Expecting \')\' after \"switch\" expression\n");
  ast_append_child(switch_stmt, parse_stmt(cxt));
  return switch_stmt;
}

token_t *parse_while_stmt(parse_stmt_cxt_t *cxt) {
  (void)cxt; return NULL;
}

token_t *parse_do_stmt(parse_stmt_cxt_t *cxt) {
  (void)cxt; return NULL;
}

token_t *parse_for_stmt(parse_stmt_cxt_t *cxt) {
  (void)cxt; return NULL;
}

token_t *parse_goto_stmt(parse_stmt_cxt_t *cxt) {
  token_t *token = token_get_next(cxt->token_cxt);
  assert(token->type == T_GOTO);
  if(token_lookahead_notnull(cxt->token_cxt, 1)->type != T_IDENT)
    error_row_col_exit(token->offset, "Expecting a label for \"goto\" statement\n");
  ast_append_child(token, token_get_next(cxt->token_cxt));
  if(!token_consume_type(cxt->token_cxt, T_SEMICOLON))
    error_row_col_exit(token->offset, "Expecting \';\' after \"goto\" statement\n");
  return token;
}

token_t *parse_brk_cont_stmt(parse_stmt_cxt_t *cxt) {
  token_t *token = token_get_next(cxt->token_cxt);
  assert(token->type == T_BREAK || token->type == T_CONTINUE);
  if(!token_consume_type(cxt->token_cxt, T_SEMICOLON))
    error_row_col_exit(token->offset, "Expecting \';\' after \"%s\" statement\n", token_symstr(token->type));
  return token;
}

token_t *parse_return_stmt(parse_stmt_cxt_t *cxt) {
  token_t *token = token_get_next(cxt->token_cxt);
  assert(token->type == T_RETURN);
  if(token_lookahead_notnull(cxt->token_cxt, 1)->type != T_SEMICOLON)
    ast_append_child(token, parse_exp(cxt, PARSE_EXP_ALLOWALL));
  if(!token_consume_type(cxt->token_cxt, T_SEMICOLON)) \
    error_row_col_exit(token->offset, "Expecting \';\' after \"return\" statement\n");
  return token;
}

// Returns a initializer list, { expr, expr, ..., expr } where expr could be nested initializer list
token_t *parse_init_list(parse_stmt_cxt_t *cxt) {
  if(!token_consume_type(cxt->token_cxt, T_LCPAREN)) 
    error_row_col_exit(cxt->token_cxt->s, "Expecting \'{\' for initializer list\n");
  token_t *list = token_alloc_type(T_INIT_LIST);
  while(1) {
    token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
    if(la->type == T_RCPAREN) { token_consume_type(cxt->token_cxt, T_RCPAREN); break; }
    if(la->type == T_LCPAREN) ast_append_child(list, parse_init_list(cxt));
    else ast_append_child(list, parse_exp(cxt, PARSE_EXP_NOCOMMA));
    // Consume the comma, and if not a comma then let the loop continue
    if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_COMMA)
      token_consume_type(cxt->token_cxt, T_COMMA);
  }
  return list;
}

token_t *parse_stmt(parse_stmt_cxt_t *cxt) {
  token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
  switch(la->type) {
    case T_DEFAULT: // Fall through
    case T_CASE: return parse_lbl_stmt(cxt, la->type);
    case T_IDENT: 
      if(token_lookahead_notnull(cxt->token_cxt, 2)->type == T_COLON) return parse_lbl_stmt(cxt, la->type);
      else return parse_exp_stmt(cxt);
    case T_LCPAREN: return parse_comp_stmt(cxt);
    case T_IF: return parse_if_stmt(cxt);
    case T_SWITCH: return parse_switch_stmt(cxt);
    case T_WHILE: return parse_while_stmt(cxt);
    case T_DO: return parse_do_stmt(cxt);
    case T_FOR: return parse_for_stmt(cxt);
    case T_GOTO: return parse_goto_stmt(cxt);
    case T_CONTINUE: return parse_brk_cont_stmt(cxt);
    case T_BREAK: return parse_brk_cont_stmt(cxt);
    case T_RETURN: return parse_return_stmt(cxt);
    default:
      parse_exp_stmt(cxt);
  }
  (void)cxt; return NULL;
}