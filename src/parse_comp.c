
#include "parse_comp.h"
#include "eval.h"

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

// Returns 1 if there is a body, 0 if no body; Name is pushed into root as either
// empty node or IDENT node. If neither name nor body is present report error
int parse_name_body(parse_comp_cxt_t *cxt, token_t *root) {
  token_t *name = token_lookahead_notnull(cxt->token_cxt, 1);
  int has_name = name->type == T_IDENT;
  ast_append_child(root, has_name ? token_get_next(cxt->token_cxt) : token_get_empty());
  int has_body = token_consume_type(cxt->token_cxt, T_LCPAREN);
  if(!has_name && !has_body) error_row_col_exit(root->offset, "Expecting identifier or \'{\' after struct/union\n");
  return has_body;
}

// Returns the same node which is either T_STRUCT or T_UNION
// The 2nd child is empty if there is no body or empty body
// Check whether decl_prop has TYPE_EMPTY_BODY bit set
token_t *parse_struct_union(parse_comp_cxt_t *cxt, token_t *root) {
  if(parse_name_body(cxt, root)) {
    
    int has_body = 0; // Might be possible that there is {} as body but it is empty
    while(1) { // loop on lines
      if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_RCPAREN) { // Finish parsing on '}'
        if(!has_body) {
          ast_append_child(root, token_get_empty());
          root->decl_prop = TYPE_EMPTY_BODY; // Distinguish this from no body defined
        }
        token_consume_type(cxt->token_cxt, T_RCPAREN); 
        break; 
      }
      has_body = 1;
      token_t *comp_decl = ast_append_child(token_alloc_type(T_COMP_DECL), parse_decl_basetype(cxt));
      while(1) { // loop on fields
        token_t *field = token_alloc_type(T_COMP_FIELD);
        ast_append_child(comp_decl, ast_append_child(field, parse_decl(cxt, PARSE_DECL_NOBASETYPE)));
        // Declarator body, can be named or unamed
        token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
        if(la->type == T_COLON) {
          token_consume_type(cxt->token_cxt, T_COLON);
          token_t *bf; // Assigned next line
          ast_append_child(field, ast_append_child(token_alloc_type(T_BITFIELD), bf = parse_exp(cxt, PARSE_EXP_NOCOMMA)));
          la = token_lookahead_notnull(cxt->token_cxt, 1);
        }
        if(la->type == T_COMMA) { token_consume_type(cxt->token_cxt, T_COMMA); }
        else if(la->type == T_SEMICOLON) { token_consume_type(cxt->token_cxt, T_SEMICOLON); break; } // Finish parsing the field on ';'
        else { error_row_col_exit(la->offset, "Unexpected symbol \"%s\" in struct/union field declaration\n", 
                                  token_typestr(la->type)); }
      }
      ast_append_child(root, comp_decl);
    }
  } else { ast_append_child(root, token_get_empty()); } // Otherwise append an empty child to indicate there is no body
  return root;
}

token_t *parse_enum(parse_comp_cxt_t *cxt, token_t *root) {
  if(parse_name_body(cxt, root)) {
    while(1) { // loop on lines
      if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_RCPAREN) { 
        token_consume_type(cxt->token_cxt, T_RCPAREN); break;
      }
      token_t *enum_field = token_alloc_type(T_ENUM_FIELD);
      ast_append_child(root, enum_field);
      token_t *la = token_lookahead_notnull(cxt->token_cxt, 1);
      if(la->type == T_IDENT) ast_append_child(enum_field, token_get_next(cxt->token_cxt));
      else error_row_col_exit(la->offset, "Expecting an identifier in enum body\n");
      la = token_lookahead_notnull(cxt->token_cxt, 1);
      if(la->type == T_ASSIGN) {
        token_consume_type(cxt->token_cxt, T_ASSIGN);
        ast_append_child(enum_field, parse_exp(cxt, PARSE_EXP_NOCOMMA));
        la = token_lookahead_notnull(cxt->token_cxt, 1);
      }
      // Last entry does not have to use comma
      if(la->type == T_COMMA) { token_consume_type(cxt->token_cxt, T_COMMA); }
      else if(la->type == T_RCPAREN) { token_consume_type(cxt->token_cxt, T_RCPAREN); break; }
      else { error_row_col_exit(la->offset, "Unexpected symbol \"%s\" in enum body\n", 
                                token_typestr(la->type)); }
    }
  }
  return root;
}