
#include "parse_comp.h"

parse_decl_cxt_t *parse_comp_init(char *input) { return parse_exp_init(input); }
void parse_comp_free(parse_comp_cxt_t *cxt) { parse_exp_free(cxt); }

// This parses struct or union or enum
token_t *parse_comp(parse_exp_cxt_t *cxt) {
  token_t *token = token_get_next(cxt->token_cxt);
  assert(token);
  switch(token->type) {
    case T_STRUCT: case T_UNION: return parse_struct_union(cxt, token);
    case T_ENUM: return parse_enum(cxt, token);
    default: assert(0);
  }
}

// Returns the same node which is either T_STRUCT or T_UNION
token_t *parse_struct_union(parse_comp_cxt_t *cxt, token_t *root) {
  token_t *name = token_lookahead_notnull(cxt->token_cxt, 1);
  int has_name = name->type == T_IDENT;
  // Name is either identifier or empty as field child
  ast_append_child(root, has_name ? token_get_next(cxt->token_cxt) : token_get_empty());
  int has_body = token_consume_type(cxt->token_cxt, T_LCPAREN);
  if(!has_name && !has_body) error_row_col_exit(root->offset, "Expecting identifier or \'{\' after struct/union\n");
  if(has_body) {
    while(1) { // loop on lines
      if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_RCPAREN) { // Finish parsing on '}'
        token_consume_type(cxt->token_cxt, T_RCPAREN); 
        break; 
      }
      token_t *comp_decl = ast_append_child(token_alloc_type(T_COMP_DECL), parse_basetype(cxt));
      while(1) { // loop on fields
        token_t *field = token_alloc_type(T_COMP_FIELD);
        parse_exp_recurse(cxt);
        ast_append_child(comp_decl, ast_append_child(field, parse_decl(cxt, PARSE_DECL_NOBASETYPE)));
        parse_exp_decurse(cxt);
        // Declarator body, can be named or unamed
        token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
        if(la->type == T_COLON) {
          token_consume_type(cxt->token_cxt, T_COLON);
          parse_exp_recurse(cxt);
          ast_append_child(field, parse_exp(cxt, PARSE_EXP_NOCOMMA));
          parse_exp_decurse(cxt);
          la = token_lookahead_notnull(cxt->token_cxt, 1);
        }
        if(la->type == T_COMMA) { token_consume_type(cxt->token_cxt, T_COMMA); }
        else if(la->type == T_SEMICOLON) { token_consume_type(cxt->token_cxt, T_SEMICOLON); break; } // Finish parsing the field on ';'
        else { error_row_col_exit(la->offset, "Unexpected symbol \"%s\" in struct/union field declaration\n", 
                                  token_typestr(la->type)); }
      }
      ast_append_child(root, comp_decl);
    }
  }
  return root;
}

token_t *parse_enum(parse_comp_cxt_t *cxt, token_t *root) {
  (void)cxt; (void)root;
  return NULL;
}