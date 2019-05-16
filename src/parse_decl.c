
#include "parse_decl.h"
#include "parse_comp.h"
#include "eval.h"

parse_decl_cxt_t *parse_decl_init(char *input) { return parse_exp_init(input); }
void parse_decl_free(parse_decl_cxt_t *cxt) { parse_exp_free(cxt); }

// Whether the token could start a declaration, i.e. being a type, modifier, or udef type
int parse_decl_isbasetype(parse_decl_cxt_t *cxt, token_t *token) { 
  (void)cxt; return ((token->decl_prop & DECL_MASK) || token->type == T_UDEF) ? 1 : 0;
}

// Same rule as parse_exp_next_token()
// Note: The following tokens are considered as part of a type expression:
//   1. ( ) [ ] *  2. const volatile 3. identifier
token_t *parse_decl_next_token(parse_decl_cxt_t *cxt) {
  token_t *token = token_lookahead(cxt->token_cxt, 1);
  int valid; // Below are not "=="
  if((valid = (token != NULL))) {
    switch(token->type) {
      case T_LPAREN: { // If the next symbol constitutes a base type then this is func call
        token_t *lookahead = token_lookahead(cxt->token_cxt, 2); // Note that we already looked ahead one token
        if(lookahead != NULL && (parse_decl_isbasetype(cxt, lookahead) || lookahead->type == T_RPAREN)) 
          token->type = EXP_FUNC_CALL;
        else token->type = EXP_LPAREN;
        break;
      }
      case T_RPAREN:
        if(parse_exp_isallowed(cxt, token, PARSE_EXP_ALLOWALL)) token->type = EXP_RPAREN;
        else valid = 0;
        break;
      case T_STAR: token->type = EXP_DEREF; break;
      case T_LSPAREN: token->type = EXP_ARRAY_SUB; break;
      case T_RSPAREN: 
        if(parse_exp_isallowed(cxt, token, PARSE_EXP_ALLOWALL)) token->type = EXP_RSPAREN;
        else valid = 0;
        break;
      case T_IDENT: break;
      //case T_ELLIPSIS: break; // Ellipsis is processed only in function decl and does not go through this function
      default: if(!(token->decl_prop & DECL_QUAL_MASK)) valid = 0; // Only allow DECL_QUAL and identifier
    }
  }
  return valid ? token_get_next(cxt->token_cxt) : NULL;
}

// Parses the type specifier part of a base type declaration
// Sets the decl_prop of the basetype node according to the type being parsed, and push child for udef, s/u/e
void parse_typespec(parse_decl_cxt_t *cxt, token_t *basetype) {
  if(BASETYPE_GET(basetype->decl_prop) != BASETYPE_NONE) 
    error_row_col_exit(cxt->token_cxt->s, "Already has type specifier \"%s\"\n", token_decl_print(basetype->decl_prop));
  int usign = 0;
  token_type_t type = token_lookahead_notnull(cxt->token_cxt, 1)->type; // Use this to detect illegal "signed long double"
  switch(type) {   // Basetype declaration cannot be the end of file
    case T_UNSIGNED: usign = 1;                                // Fall through
    case T_SIGNED: token_free(token_get_next(cxt->token_cxt)); // Fall through again
    case T_CHAR: case T_SHORT: case T_INT: case T_LONG: {      // Note: Do not get_next_token() on these types
      token_t *token = token_get_next(cxt->token_cxt);         // unsigned and signed have been processed before this line
      switch(token->type) {
        case T_CHAR: BASETYPE_SET(basetype, usign ? BASETYPE_UCHAR : BASETYPE_CHAR); token_free(token); return;
        case T_INT: BASETYPE_SET(basetype, usign ? BASETYPE_UINT : BASETYPE_INT); token_free(token); return;
        case T_SHORT: // short int has the same effect as short, so we just try to consume an extra int
          BASETYPE_SET(basetype, usign ? BASETYPE_USHORT : BASETYPE_SHORT); token_free(token); 
          token_consume_type(cxt->token_cxt, T_INT); return;
        case T_LONG: { // long long; long long int; long int; long
          token_free(token);
          token_t *token = token_get_next(cxt->token_cxt);
          switch(token->type) {
            case T_LONG: // Same as short [int]
              BASETYPE_SET(basetype, usign ? BASETYPE_ULLONG : BASETYPE_LLONG); token_free(token);
              token_consume_type(cxt->token_cxt, T_INT); return;
            case T_DOUBLE:
              if(type == T_SIGNED || type == T_UNSIGNED) 
                error_row_col_exit(token->offset, "Type \"long double\" does not allow sign declaration\n");
              BASETYPE_SET(basetype, BASETYPE_LDOUBLE); token_free(token); return;
            case T_INT: BASETYPE_SET(basetype, usign ? BASETYPE_ULONG : BASETYPE_LONG); token_free(token); return;
            default: 
              BASETYPE_SET(basetype, usign ? BASETYPE_ULONG : BASETYPE_LONG);
              token_pushback(cxt->token_cxt, token); return;
          }
        } // unsigned / signed without other base type implies int type
        default: BASETYPE_SET(basetype, usign ? BASETYPE_UINT : BASETYPE_INT); token_pushback(cxt->token_cxt, token); return;
      }
    }
    case T_FLOAT: BASETYPE_SET(basetype, BASETYPE_FLOAT); token_free(token_get_next(cxt->token_cxt)); return;
    case T_DOUBLE: BASETYPE_SET(basetype, BASETYPE_DOUBLE); token_free(token_get_next(cxt->token_cxt)); return;
    case T_UDEF: BASETYPE_SET(ast_append_child(basetype, token_get_next(cxt->token_cxt)), BASETYPE_UDEF); return;
    case T_STRUCT: BASETYPE_SET(ast_append_child(basetype, parse_comp(cxt)), BASETYPE_STRUCT); return;
    case T_UNION: BASETYPE_SET(ast_append_child(basetype, parse_comp(cxt)), BASETYPE_UNION); return;
    case T_ENUM: BASETYPE_SET(ast_append_child(basetype, parse_comp(cxt)), BASETYPE_ENUM); return;
    case T_VOID: BASETYPE_SET(basetype, BASETYPE_VOID); token_free(token_get_next(cxt->token_cxt)); return;
    default: assert(0);
  }
}

// Base type = one of udef/builtin/enum/struct/union; In this stage only allows 
// keywords with TOKEN_DECL set
// The stack is not changed, calling this function does not need recurse
token_t *parse_decl_basetype(parse_decl_cxt_t *cxt) {
  token_t *token = token_lookahead(cxt->token_cxt, 1), *basetype = token_alloc_type(T_BASETYPE);
  while(token != NULL && (token->decl_prop & DECL_MASK)) {
    if(!(token->decl_prop & DECL_TYPE_MASK)) {
      if(!token_decl_apply(basetype, token)) 
        error_row_col_exit(token->offset, "Incompatible type modifier \"%s\" with \"%s\"\n",
        token_symstr(token->type), token_decl_print(basetype->decl_prop));
      token_consume_type(cxt->token_cxt, token->type); // Consume whatever it is
    } else { parse_typespec(cxt, basetype); }
    token = token_lookahead(cxt->token_cxt, 1);
  } // Must have some type, cannot be just qualifiers and modifiers
  if(BASETYPE_GET(basetype->decl_prop) == BASETYPE_NONE) error_row_col_exit(cxt->token_cxt->s, "Declaration lacks a type specifier\n");
  return basetype;
}

token_t *parse_decl(parse_decl_cxt_t *cxt, int hasbasetype) {
  parse_exp_recurse(cxt);
  assert(parse_exp_size(cxt, OP_STACK) == 0 && parse_exp_size(cxt, AST_STACK) == 0); // Must start on a new stack
  token_t *decl = token_alloc_type(T_DECL);
  // Append base type node if the flag indicates so, or empty node as placeholder
  ast_append_child(decl, hasbasetype == PARSE_DECL_HASBASETYPE ? parse_decl_basetype(cxt) : token_get_empty()); 
  token_t *placeholder = token_get_empty();
  // Placeholder operand for the innremost operator because we do not push ident to AST stack
  parse_exp_shift(cxt, AST_STACK, placeholder); 
  token_t *decl_name = NULL;  // If not an abstract declarator this is the name
  while(1) {
    token_t *token = parse_decl_next_token(cxt);
    if(token == NULL) {
      ast_append_child(decl, parse_exp_reduce_all(cxt)); // This may directly put the placeholder node as a expression
      ast_append_child(decl, decl_name ? decl_name : token_get_empty()); // Only appends the name if there is one, or empty node
      parse_exp_decurse(cxt);
      // Two cases: (1) If the parent of placeholder node is T_DECL then there is no expression, in which 
      // case we retain the empty node to indicate no expression; (2) Otherwise there is an expression
      // and we just remove the empty node (the exp has no concrete operand at leaf level) and free it
      //if(placeholder->parent->type != T_DECL) token_free(ast_remove(placeholder)); 
      return decl;
    }
    if(token->decl_prop & DECL_QUAL_MASK) { // Special case for type qualifiers
      token_t *top = parse_exp_peek(cxt, OP_STACK);
      if(top == NULL || top->type != EXP_DEREF || cxt->last_active_stack != OP_STACK) 
        error_row_col_exit(token->offset, "Qualifier \"%s\" must follow pointer\n", token_symstr(token->type));
      if(!token_decl_apply(top, token))
        error_row_col_exit(token->offset, "Qualifier \"%s\" not compatible with \"%s\"\n",
                           token_symstr(token->type), token_decl_print(top->decl_prop));
      token_free(token);
    } else {
      switch(token->type) {
        case EXP_DEREF: // To avoid int **a*; being legal, because identifiers are not pushed to AST stack
          if(decl_name) error_row_col_exit(token->offset, "Pointers can only occur before declared name\n") 
          parse_exp_shift(cxt, OP_STACK, token); break;
        case T_IDENT:  // Trick: Do not push it onto the stack
          if(decl_name) error_row_col_exit(token->offset, "Type declaration can have at most one identifier\n");
          decl_name = token; break;
          /* Is the above sufficient? - As long as parenthesis is not parsed recursively it is fine
          token_t *ast_top = parse_exp_peek(cxt, AST_STACK);
          if(ast_top != NULL && ast_top->type == T_) token_free(stack_pop(cxt->stacks[AST_STACK]));
          else if(ast_top != NULL) error_row_col_exit(token->offset, "Type declaration can have at most one identifier\n");
          parse_exp_shift(cxt, AST_STACK, token);
          */
        case EXP_ARRAY_SUB: {
          parse_exp_shift(cxt, OP_STACK, token);
          token_t *la = token_lookahead(cxt->token_cxt, 1);
          token_t *index;
          if(la != NULL && la->type == T_RSPAREN) { index = token_get_empty(); }
          else { index = parse_exp(cxt, PARSE_EXP_ALLOWALL); }
          parse_exp_shift(cxt, AST_STACK, index);
          parse_exp_reduce(cxt, -1, 1); // This reduces array sub
          if(!token_consume_type(cxt->token_cxt, T_RSPAREN)) 
            error_row_col_exit(token->offset, "Array declaration expects \']\'\n");
          // Evaluate the constant integer expression and store array size in token->array_size (-1 if none specified)
          if(index->type != T_) {
            token->array_size = eval_const_int(index);
            if(token->array_size < 0) error_row_col_exit(token->offset, "Array size in declaration must be non-negative\n");
          } else { token->array_size = -1; }
          break;
        }
        case EXP_FUNC_CALL: {
          parse_exp_shift(cxt, OP_STACK, token);
          token_t *la = token_lookahead(cxt->token_cxt, 1);
          if(la != NULL && la->type == T_RPAREN) {
            ast_push_child(token, token_get_empty());
            token_consume_type(cxt->token_cxt, T_RPAREN);
          } else {
            while(1) {
              ast_append_child(token, parse_decl(cxt, PARSE_DECL_HASBASETYPE));
              if(token_consume_type(cxt->token_cxt, T_COMMA)) { // Special: check "..." after ","
                if(token_lookahead_notnull(cxt->token_cxt, 1)->type == T_ELLIPSIS) { // after '...' there can only be ')'
                  ast_append_child(token, token_get_next(cxt->token_cxt));
                  if(!token_consume_type(cxt->token_cxt, T_RPAREN))
                    error_row_col_exit(cxt->token_cxt->s, "\"...\" could only be the last function argument\n");
                  break;
                }
              }
              else if(token_consume_type(cxt->token_cxt, T_RPAREN)) { break; }
              else error_row_col_exit(token->offset, "Function declaration expects \')\' or \',\' or \"...\"\n");
            }
          }
          parse_exp_reduce(cxt, 1, 1); // This reduces EXP_FUNC_CALL
          break;
        }
        case EXP_LPAREN: parse_exp_shift(cxt, OP_STACK, token); break;
        case EXP_RPAREN: {
          token_t *op_top = parse_exp_peek(cxt, OP_STACK);
          while(op_top != NULL && op_top->type != EXP_LPAREN) op_top = parse_exp_reduce(cxt, -1, 0);
          if(op_top == NULL) error_row_col_exit(token->offset, "Did not find matching \'(\' in declaration\n");
          token_free(stack_pop(cxt->stacks[OP_STACK]));
          token_free(token);
          break;
        } // Note that unrelated tokens are filtered
        default: printf("%s %s\n", token_typestr(token->type), token->offset); assert(0);
      } // switch(token->type)
    } // if(token is qualifier)
  } // while(1)
}
