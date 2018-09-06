
#include "parse_exp.h"
#include "parse_decl.h"
#include "error.h"

parse_exp_cxt_t *parse_exp_init(char *input) {
  parse_exp_cxt_t *cxt = (parse_exp_cxt_t *)malloc(sizeof(parse_exp_cxt_t));
  if(cxt == NULL) perror(__func__);
  cxt->stacks[0] = stack_init();
  cxt->stacks[1] = stack_init();
  cxt->tops[0] = stack_init();
  cxt->tops[1] = stack_init();
  parse_exp_recurse(cxt);
  // If the first token is an operator then it must be prefix operator
  cxt->last_active_stack = OP_STACK;
  cxt->token_cxt = token_cxt_init(input);
  // Enable error reporting
  error_init(input);
  return cxt;
}

void parse_exp_free(parse_exp_cxt_t *cxt) {
  stack_free(cxt->stacks[0]);
  stack_free(cxt->stacks[1]);
  stack_free(cxt->tops[0]);
  stack_free(cxt->tops[1]);
  token_cxt_free(cxt->token_cxt);
  free(cxt);
  return;
}

// Whether the current level is the outter most, i.e. not inside any ( or [
int parse_exp_outermost(parse_exp_cxt_t *cxt) {
  int found = 0;
  for(int i = 0;i < parse_exp_size(cxt, OP_STACK);i++) {
    token_type_t type = parse_exp_peek_at(cxt, OP_STACK, i)->type;
    if(type == EXP_FUNC_CALL || type == EXP_LPAREN || type == EXP_ARRAY_SUB) { found = 1; break; }
  }
  return !found;
}

// Whether a ) or ] belongs to the current level. If no matching ( or [ is found
// then it is not a closing
int parse_exp_hasmatch(parse_exp_cxt_t *cxt, token_t *token) {
  return (token->type == T_RPAREN || token->type == T_RSPAREN) ? !parse_exp_outermost(cxt) : 1;
}

// Determine if a token could continue an expression currently being parsed
// Literals (incl. ident), operators, and sizeof() could be part of an expression
int parse_exp_isexp(parse_exp_cxt_t *cxt, token_t *token) {
  token_type_t type = token->type;
  // For closing symbols, i.e. ) and ], there must be a matching ( or [
  if(!parse_exp_hasmatch(cxt, token)) return 0; 
  return ((type >= T_OP_BEGIN && type < T_OP_END) || 
          (type >= T_LITERALS_BEGIN && type < T_LITERALS_END) || 
          (type == T_SIZEOF));
}

// Returns whether a token is primary token
int parse_exp_isprimary(parse_exp_cxt_t *cxt, token_t *token) {
  return token->type >= T_LITERALS_BEGIN && token->type < T_LITERALS_END; (void)cxt;
}

// Returns whether the next lookahead token is a type
int parse_exp_la_isdecl(parse_exp_cxt_t *cxt) {
  token_t *token = token_lookahead(cxt->token_cxt, 1);
  if(token == NULL || !parse_decl_isbasetype(cxt, token)) return 0;
  return 1;
}

// Virtual size of the stack, which is the difference between the previous top and the current top
int parse_exp_size(parse_exp_cxt_t *cxt, int stack_id) {
  return stack_topaddr(cxt->stacks[stack_id]) - (void **)stack_peek(cxt->tops[stack_id]);
}

// Returns NULL if stack empty, or stack top
token_t *parse_exp_peek(parse_exp_cxt_t *cxt, int stack_id) {
  return parse_exp_isempty(cxt, stack_id) ? NULL : (token_t *)stack_peek(cxt->stacks[stack_id]);
}

token_t *parse_exp_peek_at(parse_exp_cxt_t *cxt, int stack_id, int index) {
  return parse_exp_isempty(cxt, stack_id) ? NULL : (token_t *)stack_peek_at(cxt->stacks[stack_id], index);
}

// Whether the stack is empty
int parse_exp_isempty(parse_exp_cxt_t *cxt, int stack_id) {
  return parse_exp_size(cxt, stack_id) == 0;
}

// Creates a new level of virtual stack
void parse_exp_recurse(parse_exp_cxt_t *cxt) {
  stack_push(cxt->tops[0], stack_topaddr(cxt->stacks[0]));
  stack_push(cxt->tops[1], stack_topaddr(cxt->stacks[1]));
  return;
}

void parse_exp_decurse(parse_exp_cxt_t *cxt) {
  assert((void **)stack_peek(cxt->tops[0]) == stack_topaddr(cxt->stacks[0]));
  assert((void **)stack_peek(cxt->tops[1]) == stack_topaddr(cxt->stacks[1]));
  stack_pop(cxt->tops[0]); stack_pop(cxt->tops[1]);
}

// Returned token is allocated from the heap, caller free
// If the token does not belong to expressions, or we reached the end then 
// return NULL
token_t *parse_exp_next_token(parse_exp_cxt_t *cxt) {
  token_t *token = token_lookahead(cxt->token_cxt, 1);
  if(token == NULL || !parse_exp_isexp(cxt, token)) return NULL;
  else token = token_get_next(cxt->token_cxt);
  // Initialize AST pointers before we use it to build AST
  ast_make_node(token);
  // Primary expressions are not considered for operator type deciding
  if(parse_exp_isprimary(cxt, token)) return token;

  if(cxt->last_active_stack == AST_STACK) {
    // If the last active stack is AST stack, then the op must be postfix (unary)
    // or binary
    switch(token->type) {
      case T_LPAREN: token->type = EXP_FUNC_CALL; break;
      case T_RPAREN: token->type = EXP_RPAREN; break;
      case T_LSPAREN: token->type = EXP_ARRAY_SUB; break;
      case T_RSPAREN: token->type = EXP_RSPAREN; break;
      case T_DOT: token->type = EXP_DOT; break;
      case T_ARROW: token->type = EXP_ARROW; break;
      case T_INC: token->type = EXP_POST_INC; break;
      case T_DEC: token->type = EXP_POST_DEC; break;
      case T_PLUS: token->type = EXP_ADD; break;
      case T_MINUS: token->type = EXP_SUB; break;
      //case T_LOGICAL_NOT: token->type = EXP_; break;
      //case T_BIT_NOT: token->type = EXP_; break;
      case T_STAR: token->type = EXP_MUL; break;
      case T_AND: token->type = EXP_BIT_AND; break;
      //case T_SIZEOF: token->type = EXP_; break;
      case T_DIV: token->type = EXP_DIV; break;
      case T_MOD: token->type = EXP_MOD; break;
      case T_LSHIFT: token->type = EXP_LSHIFT; break;
      case T_RSHIFT: token->type = EXP_RSHIFT; break;
      case T_LESS: token->type = EXP_LESS; break;
      case T_GREATER: token->type = EXP_GREATER; break;
      case T_LEQ: token->type = EXP_LEQ; break;
      case T_GEQ: token->type = EXP_GEQ; break;
      case T_EQ: token->type = EXP_EQ; break;
      case T_NEQ: token->type = EXP_NEQ; break;
      case T_BIT_XOR: token->type = EXP_BIT_XOR; break;
      case T_BIT_OR: token->type = EXP_BIT_OR; break;
      case T_LOGICAL_AND: token->type = EXP_LOGICAL_AND; break;
      case T_LOGICAL_OR: token->type = EXP_LOGICAL_OR; break;
      case T_QMARK: token->type = EXP_COND; break;
      case T_COLON: token->type = EXP_COLON; break;
      case T_ASSIGN: token->type = EXP_ASSIGN; break;
      case T_PLUS_ASSIGN: token->type = EXP_ADD_ASSIGN; break;
      case T_MINUS_ASSIGN: token->type = EXP_SUB_ASSIGN; break;
      case T_MUL_ASSIGN: token->type = EXP_MUL_ASSIGN; break;
      case T_DIV_ASSIGN: token->type = EXP_DIV_ASSIGN; break;
      case T_MOD_ASSIGN: token->type = EXP_MOD_ASSIGN; break;
      case T_LSHIFT_ASSIGN: token->type = EXP_LSHIFT_ASSIGN; break;
      case T_RSHIFT_ASSIGN: token->type = EXP_RSHIFT_ASSIGN; break;
      case T_AND_ASSIGN: token->type = EXP_AND_ASSIGN; break;
      case T_OR_ASSIGN: token->type = EXP_OR_ASSIGN; break;
      case T_XOR_ASSIGN: token->type = EXP_XOR_ASSIGN; break;
      case T_COMMA: token->type = EXP_COMMA; break;
      default: error_row_col_exit(token->offset, 
                                  "Did not expect to see \"%s\" as a postfix operator\n", 
                                  token_symstr(token->type));
    }
  } else {
    // If the last active stack is operator stack then it must be an unary prefix operator
    switch(token->type) {
      case T_LPAREN: token->type = EXP_LPAREN; break;        // Ordinary parenthesis or type cast
      case T_RPAREN: token->type = EXP_RPAREN; break;        // ( exp... )
      // Postfix ++ and -- must be reduced immediately because they have the highest precedence
      case T_INC: token->type = EXP_PRE_INC; break;
      case T_DEC: token->type = EXP_PRE_DEC; break;
      case T_PLUS: token->type = EXP_PLUS; break;
      case T_MINUS: token->type = EXP_MINUS; break;
      case T_LOGICAL_NOT: token->type = EXP_LOGICAL_NOT; break;
      case T_BIT_NOT: token->type = EXP_BIT_NOT; break;
      case T_STAR: token->type = EXP_DEREF; break;
      case T_AND: token->type = EXP_ADDR; break;
      case T_SIZEOF: token->type = EXP_SIZEOF; break;
      default: error_row_col_exit(token->offset, 
                                  "Did not expect to see \"%s\" as a prefix operator\n", 
                                  token_symstr(token->type));
    }
  }
  return token;
}

void parse_exp_shift(parse_exp_cxt_t *cxt, int stack_id, token_t *token) {
  assert(stack_id == OP_STACK || stack_id == AST_STACK);
  stack_push(cxt->stacks[stack_id], token);
  cxt->last_active_stack = stack_id;
  if(stack_id == AST_STACK) {
    switch(token->type) {
      case EXP_FUNC_CALL: ast_collect_funcarg(token); break; // For decl this function does nothing
      case EXP_COND: ast_movecond(token); break;
      default: break;
    }
  }
  return;
}

// One reduce step of the topmost operator
// Return the next op at stack top; NULL if stack empty
// If op stack is empty then do nothing, and return NULL
// If the override is -1 then we ignore it
// If allow_paren is set then [ and ( can be reduced
token_t *parse_exp_reduce(parse_exp_cxt_t *cxt, int op_num_override, int allow_paren) {
  stack_t *ast = cxt->stacks[AST_STACK], *op = cxt->stacks[OP_STACK];
  if(parse_exp_isempty(cxt, OP_STACK)) return NULL;
  token_t *top_op = stack_pop(op);
  // Note that '[' and '(' are reduced manually, and this function could not reduce them
  // Otherwise ( and [ may not be balanced, e.g. (a[0) would be allowed
  if(!allow_paren && 
     (top_op->type == EXP_FUNC_CALL || top_op->type == EXP_LPAREN || top_op->type == EXP_ARRAY_SUB)) 
    error_row_col_exit(top_op->offset, "Symbol \"%s\" unclosed\n", token_typestr(top_op->type));
  int op_num = op_num_override == -1 ? token_get_num_operand(top_op->type) : op_num_override;
  for(int i = 0;i < op_num;i++) {
    if(parse_exp_isempty(cxt, AST_STACK))
      error_row_col_exit(top_op->offset, "Wrong number of operands for operator %s\n", 
                         token_typestr(top_op->type));
    token_t *operand = stack_pop(ast);
    // Note that nodes are poped in reverse order
    ast_push_child(top_op, operand);
  }

  parse_exp_shift(cxt, AST_STACK, top_op);
  return parse_exp_isempty(cxt, OP_STACK) ? NULL : stack_peek(op);
}

// Reduce until the precedence of the stack top is less than (or equal to, depending 
// on the associativity) the given token, or the stack becomes empty
// Note:
//   1. Higher precedence has lower numerical number
//   2. Precedence reduction must not cross array sub and function call, otherwise 
//      expression a[b--] will reduce a[b first and then -- because they are both precedence 0
//      and are left-associative
void parse_exp_reduce_preced(parse_exp_cxt_t *cxt, token_t *token) {
  int preced; assoc_t assoc;
  int top_preced; assoc_t top_assoc;
  token_get_property(token->type, &preced, &assoc);
  token_t *op_stack_top = parse_exp_isempty(cxt, OP_STACK) ? NULL : (token_t *)stack_peek(cxt->stacks[OP_STACK]);
  while(op_stack_top != NULL && op_stack_top->type != EXP_FUNC_CALL && 
        op_stack_top->type != EXP_ARRAY_SUB && op_stack_top->type != EXP_LPAREN) {
    token_get_property(op_stack_top->type, &top_preced, &top_assoc);
    if(preced < top_preced || 
       ((preced == top_preced) && (assoc == ASSOC_RL))) break;
    op_stack_top = parse_exp_reduce(cxt, -1, 0);
    printf("Reduce %s\n", token_typestr(parse_exp_peek(cxt, AST_STACK)->type));
  }
  return;
}

// Reduce as much as we can and pop the only element from the AST stack
// Report error if in the final state there are more than one element on AST stack
// or any element on operator stack
token_t *parse_exp_reduce_all(parse_exp_cxt_t *cxt) {
  while(parse_exp_reduce(cxt, -1, 0) != NULL);
  if(!parse_exp_isempty(cxt, OP_STACK)) {
    error_row_col_exit(parse_exp_peek(cxt, OP_STACK)->offset,
                       "Did not find operand for operator %s\n", 
                       token_typestr(parse_exp_peek(cxt, OP_STACK)->type));
  } else if(parse_exp_size(cxt, AST_STACK) != 1) {
    error_row_col_exit(parse_exp_peek(cxt, AST_STACK)->offset,
                       "Missing operator for the entity\n");
  } 
  return (token_t *)stack_pop(cxt->stacks[AST_STACK]);
}

token_t *parse_exp(parse_exp_cxt_t *cxt) {
  stack_t *op = cxt->stacks[OP_STACK];
  while(1) {
    token_t *token = parse_exp_next_token(cxt);
    if(token == NULL) {
      return parse_exp_reduce_all(cxt);
    } else if(parse_exp_isprimary(cxt, token)) {
      parse_exp_shift(cxt, AST_STACK, token);
    } else if(token->type == EXP_RPAREN) {   // All tokens below this line are operators
      token_t *op_top = parse_exp_peek(cxt, OP_STACK);
      // Special case: function with no argument; must be the case that a FUNC_CALL '(' is 
      // pushed immediately followed by ')'
      if(op_top != NULL && cxt->last_active_stack == OP_STACK && op_top->type == EXP_FUNC_CALL) {
        parse_exp_shift(cxt, AST_STACK, token_get_empty());
        parse_exp_reduce(cxt, -1, 1); // This reduces no argument EXP_FUNC_CALL
      } else {
        while(op_top != NULL && op_top->type != EXP_FUNC_CALL && op_top->type != EXP_LPAREN) 
          op_top = parse_exp_reduce(cxt, -1, 0);
        if(op_top == NULL) { error_row_col_exit(token->offset, "Did not find matching \'(\'\n"); }
        else if(op_top->type == EXP_LPAREN) { token_free(stack_pop(op)); } // Left paren is not used in AST
        else { parse_exp_reduce(cxt, -1, 1); } // This reduces EXP_FUNC_CALL
      }
      token_free(token); // Right paren is always not used in AST
    } else if(token->type == EXP_RSPAREN) {
      token_t *op_top = parse_exp_peek(cxt, OP_STACK);
      while(op_top != NULL && op_top->type != EXP_ARRAY_SUB) op_top = parse_exp_reduce(cxt, -1, 0);
      if(op_top == NULL) error_row_col_exit(token->offset, "Did not find matching \'[\'\n");
      parse_exp_reduce(cxt, -1, 1); // This reduces '['
      token_free(token);
    } else if(token->type == EXP_LPAREN && parse_exp_la_isdecl(cxt)) {
      token->type = EXP_CAST;
      parse_exp_recurse(cxt);
      token_t *decl = parse_decl(cxt);
      parse_exp_decurse(cxt);
      ast_push_child(token, decl);
      parse_exp_shift(cxt, OP_STACK, token);
      if(!token_consume_type(cxt->token_cxt, T_RPAREN)) 
          error_row_col_exit(token->offset, "Type cast expects \')\'\n");
    } else {
      parse_exp_reduce_preced(cxt, token);
      parse_exp_shift(cxt, OP_STACK, token);
      // Special care must be taken for postfix ++ and -- because they cause the 
      // parser to think an op has been pushed and all following are unary prefix op
      // We need to reduce immediately upon seeing them. This does not affect correctness
      // because these two have the highest priority
      if(token->type == EXP_POST_DEC || token->type == EXP_POST_INC) parse_exp_reduce(cxt, -1, 0);
    }
  }
}