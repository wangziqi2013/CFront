

#include "parse_exp.h"
#include "parse_decl.h"

parse_decl_cxt_t *parse_decl_init(char *input) { return parse_exp_init(input); }
void parse_decl_free(parse_decl_cxt_t *cxt) { parse_exp_free(cxt); }

// Whether the token could start a declaration, i.e. being a type or modifier
int parse_decl_isbasetype(parse_decl_cxt_t *cxt, token_t *token) {
  if(token->decl_prop & DECL_MASK) return 1; // built-in type & modifier
  else if(token->type == T_UDEF) return 1; // udef types
  return 0; (void)cxt;
}

// Same rule as parse_exp_next_token()
// Note: The following tokens are considered as part of a type expression:
//   1. ( ) [ ] *  2. const volatile 3. identifier
token_t *parse_decl_next_token(parse_decl_cxt_t *cxt) {
  token_t *token = token_get_next(cxt->token_cxt);
  int valid = 1;
  if(token == NULL || 
     (parse_exp_isempty(cxt, OP_STACK) && (token->type == T_RPAREN || token->type == T_RSPAREN))) {
    valid = 0;
  } else {
    switch(token->type) {
      case T_LPAREN: {  // If the next symbol constitutes a base type then this is func call
        token_t *lookahead = token_lookahead(cxt->token_cxt, 1);
        if(lookahead != NULL && (parse_decl_isbasetype(cxt, lookahead) || lookahead->type == T_RPAREN)) 
          token->type = EXP_FUNC_CALL;
        else token->type = EXP_LPAREN;
        break;
      }
      case T_RPAREN: {
        if(parse_exp_hasmatch(cxt, token)) token->type = EXP_RPAREN;
        else valid = 0;
        printf("%d\n", parse_exp_hasmatch(cxt, token));
        break;
      } 
      case T_STAR: token->type = EXP_DEREF; break;
      case T_LSPAREN: token->type = EXP_ARRAY_SUB; break;
      case T_RSPAREN: {
        if(parse_exp_hasmatch(cxt, token)) token->type = EXP_RSPAREN;
        else valid = 0;
        break;
      } 
      case T_IDENT: break;
      default: // Only allow DECL_QUAL and identifier
        if(!(token->decl_prop & DECL_QUAL_MASK)) valid = 0;
    }
  }
  if(!valid) {
    if(token != NULL) token_pushback(cxt->token_cxt, token);
    return NULL;
  } else return token;
}

// Base type = one of udef/builtin/enum/struct/union; In this stage only allows 
// keywords with TOKEN_DECL set
// The stack is not changed
void parse_basetype(parse_decl_cxt_t *cxt, token_t *basetype) {
  token_t *token = token_get_next(cxt->token_cxt);
  assert(basetype->type == T_BASETYPE);
  while(token != NULL && (token->decl_prop & DECL_MASK)) {
    if(!token_decl_compatible(token, basetype->decl_prop)) 
      error_row_col_exit(token->offset, "Incompatible type modifier \"%s\" with \"%s\"\n",
      token_symstr(token->type), token_decl_print(basetype->decl_prop));
    basetype->decl_prop = token_decl_apply(token, basetype->decl_prop);
    if(token->type == T_STRUCT || token->type == T_UNION || token->type == T_ENUM) {
      assert(0); // TODO: PARSE STRUCT ENUM UNION
      // token = parse_cmpsit(cxt, ...);
    }
    ast_append_child(basetype, token);
    token = token_get_next(cxt->token_cxt);
  }
  if(token != NULL) token_pushback(cxt->token_cxt, token);
  return;
}

token_t *parse_decl(parse_decl_cxt_t *cxt) {
  assert(parse_exp_size(cxt, OP_STACK) == 0 && parse_exp_size(cxt, AST_STACK) == 0);
  // Artificial node that is not in the token stream
  token_t *basetype = token_alloc(), *decl = token_alloc();
  basetype->type = T_BASETYPE, decl->type = T_DECL;
  ast_append_child(decl, basetype);
  parse_basetype(cxt, basetype);
  //return decl;
  // Creates an empty node and shift it into OP stack
  token_t *empty = token_alloc();
  empty->type = T_;
  parse_exp_shift(cxt, AST_STACK, empty);
  while(1) {
    token_t *token = parse_decl_next_token(cxt);
    if(token == NULL) return ast_append_child(decl, parse_exp_reduce_all(cxt));
    if(token->decl_prop & DECL_QUAL_MASK) { // Special case for type qualifiers
      token_t *top = parse_exp_peek(cxt, OP_STACK);
      if(top == NULL || top->type != EXP_DEREF || cxt->last_active_stack != OP_STACK) 
        error_row_col_exit(token->offset, "Qualifier \"%s\" must modify pointer\n", token_symstr(token->type));
      if(!token_decl_compatible(token, top->decl_prop))
        error_row_col_exit(token->offset, "Qualifier \"%s\" not compatible with \"%s\"\n",
                           token_symstr(token->type), token_decl_print(top->decl_prop));
      top->decl_prop = token_decl_apply(token, top->decl_prop);
      ast_push_child(top, token);
      continue;
    }
    switch(token->type) {
      case EXP_DEREF: parse_exp_shift(cxt, OP_STACK, token); break;
      case T_IDENT: {
        token_t *op_top = parse_exp_peek(cxt, AST_STACK);
        if(op_top != NULL && op_top->type == T_) token_free(stack_pop(cxt->stacks[AST_STACK]));
        else if(op_top != NULL) error_row_col_exit(token->offset, "Type declaration can have at most one identifier\n");
        parse_exp_shift(cxt, AST_STACK, token);
        break;
      } 
      case EXP_ARRAY_SUB: {
        parse_exp_shift(cxt, OP_STACK, token);
        token_t *la = token_lookahead(cxt->token_cxt, 1);
        token_t *index;
        if(la != NULL && la->type == T_RSPAREN) {
          index = token_alloc();
          index->type = T_;
        } else {
          parse_exp_recurse(cxt);
          index = parse_exp(cxt);
          parse_exp_decurse(cxt);
        }
        parse_exp_shift(cxt, AST_STACK, index);
        parse_exp_reduce(cxt, -1);
        if(!token_consume_type(cxt->token_cxt, T_RSPAREN)) 
          error_row_col_exit(token->offset, "Array declaration expects \']\'\n");
        break;
      }
      case EXP_FUNC_CALL: { /*
        parse_exp_shift(cxt, OP_STACK, token);
        token_t *la = token_lookahead(cxt->token_cxt, 1);
        token_t *args;
        if(la != NULL && la->type == T_RPAREN) {
          index = token_alloc();
          index->type = T_;
        } else {
          parse_exp_recurse(cxt);
          index = parse_decl(cxt);
          parse_exp_decurse(cxt);
        }
        parse_exp_shift(cxt, AST_STACK, index);
        parse_exp_reduce(cxt, -1);
        token_t *temp = token_get_next(cxt->token_cxt);
        if(temp != NULL && temp->type == T_RSPAREN) token_free(temp);
        else { 
          if(temp == NULL) { error_row_col_exit(token->offset, "Array declaration unclosed\n"); }
          else { error_row_col_exit(temp->offset, "Array declaration expects \']\', not \'%s\'\n", token_typestr(temp->type)); }
        }
        break;*/
      }
      default: printf("%s %s\n", token_typestr(token->type), token->offset); assert(0);
    }
  }
}