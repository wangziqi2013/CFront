

#include "parse_exp.h"
#include "parse_decl.h"

parse_decl_cxt_t *parse_decl_init(char *input) { return parse_exp_init(input); }
void parse_decl_free(parse_decl_cxt_t *cxt) { parse_exp_free(cxt); }

// Whether the token could start a declaration, i.e. being a type or modifier
int parse_decl_istype(parse_decl_cxt_t *cxt, token_t *token) {
  if(token->decl_prop & DECL_MASK) return 1; // built-in type & modifier
  else if(token->type == T_UDEF) return 1; // udef types
  return 0;
}

// Same rule as parse_exp_next_token()
// Note: The following tokens are considered as part of a type expression:
//   1. ( ) [ ] *  2. specifiers, qualifiers and types 3. typedef'ed names
token_t *parse_decl_next_token(parse_decl_cxt_t *cxt) {
  token_t *token = token_get_next(cxt->token_cxt);
  int valid = 1;
  if(token == NULL || 
     (parse_exp_isempty(cxt, OP_STACK) && (token->type == T_RPAREN || token->type == T_RSPAREN)))
    valid = 0;
  else
    switch(token->type) {
      case T_LPAREN:   // The only symbol that can have two meanings
        token->type = cxt->last_active_stack == OP_STACK ? EXP_LPAREN : EXP_FUNC_CALL; break;
      case T_RPAREN: token->type = EXP_RPAREN; break;
      case T_STAR: token->type = EXP_DEREF; break;
      case T_LSPAREN: token->type = EXP_ARRAY_SUB; break;
      case T_RSPAREN: token->type = EXP_RSPAREN; break;
      default: // For keywords and other symbols. Only allow DECL keywords (see token.h)
        if(!(token->decl_prop & DECL_MASK)) valid = 0;
    }
  if(!valid) {
    token_pushback(cxt->token_cxt, token);
    return NULL;
  }
  return token;
}

// Could be at most one AST on the current virtual stack
void parse_decl_shift(parse_decl_cxt_t *cxt, int stack_id, token_t *token) {
  parse_exp_shift(cxt, stack_id, token);
}

token_t *parse_decl_reduce(parse_decl_cxt_t *cxt, token_t *root) {
  if(parse_exp_size(cxt, OP_STACK) == 0) return NULL;
  token_t *top = stack_pop(cxt->stacks[OP_STACK]);
  if(parse_exp_size(cxt, AST_STACK) == 0) { // Unnamed declaration
    if(top->type == EXP_DEREF) {
      assert(root->type == T_DECL);
      root->type = T_ABS_DECL;
    } else {
      error_row_col_exit(top->offset, "")
    }
  }
  

}

// Base type = one of udef/builtin/enum/struct/union; In this stage only allows 
// keywords with TOKEN_DECL set
// The stack is not changed
void parse_basetype(parse_decl_cxt_t *cxt, token_t *basetype) {
  token_t *token = token_get_next(cxt->token_cxt);
  assert(basetype->type == T_BASETYPE);
  while(token != NULL && (token->decl_prop & DECL_MASK)) {
    if(!token_decl_compatible(token, basetype->decl_prop)) 
      error_row_col_exit(token_offset, "Incompatible type modifier \"%s\" with \"%s\"\n",
      token->str, token_decl_print(basetype->decl_prop));
    basetype->decl_prop = token_decl_apply(token, basetype->decl_prop);
    if(token->type == T_STRUCT || token->type == T_UNION || token->type == T_ENUM) {
      assert(0); // TODO: PARSE STRUCT ENUM UNION
      // token = parse_cmpsit(cxt, ...);
    }
    ast_append_child(basetype, token);
    token = token_get_next(cxt->token_cxt);
  }
  if(token != NULL) token_pushback(cxt, token);
  return;
}

token_t *parse_decl(parse_decl_cxt_t *cxt) {
  assert(parse_exp_size(cxt, OP_STACK) == 0 && parse_exp_size(cxt, AST_STACK) == 0);
  // Artificial node that is not in the token stream
  token_t *basetype = token_alloc(), *decl = token_alloc();
  basetype->type = T_BASETYPE, decl->type = T_DECL;
  ast_append_child(decl, basetype);
  parse_basetype(cxt, basetype);
  while(1) {
    token_t *token = parse_decl_next_token(cxt);
    if(token == NULL) {
      // TODO: REDUCE UNTIL STACK EMPTY
      // TODO: RETURN THE LAST TOKEN
    }
    token_t *top = stack_peek(cxt->stacks[OP_STACK]);
    if(token->decl_prop & DECL_MASK) {
      if(top->type == EXP_DEREF && !(token->decl_prop & DECL_QUAL_MASK)) 
        error_row_col_exit(token->offset, "Type specifier \"%s\" cannot be applied to \'*\'\n", 
                           token_symstr(token->type));
      decl_prop_t after = token_decl_apply(token, top->decl_prop);
      if(after == DECL_INVALID) 
        error_row_col_exit(token->offset, 
                            "Incompatible type specifier \"%s\" with declaration \"%s\"\n", 
                            token->str, token_decl_print(top->decl_prop));
      top->decl_prop = after;
      if(token->type == T_STRUCT || token->type == T_UNION || token->type == T_ENUM) {
        // TODO: EXTRA PROCESSING
        assert(0);
      } else {
        token_free(token);  // Have been applied to the bit mask, no longer useful
      }
    }
    switch(top->type) {
      case T_DECL: { 
         if(token->type == EXP_DEREF) {
          parse_exp_shift(cxt, OP_STACK, token);
        } else if(0) {
          // process [
        } else if(0) {
          //if(ast == NULL) {}
        }
      }
      case EXP_DEREF: break;
    }
  }
}