
#include "parse_exp.h"

parse_exp_cxt_t *parse_exp_init(char *input) {
  parse_exp_cxt_t *cxt = (parse_exp_cxt_t *)malloc(sizeof(parse_exp_cxt_t));
  if(cxt == NULL) perror(__func__);
  cxt->stacks[0] = stack_init();
  cxt->stacks[1] = stack_init();
  // If the first token is an operator then it must be prefix operator
  cxt->last_active_stack = OP_STACK;
  cxt->s = input;
  cxt->allow_comma = 1;
  return cxt;
}

void parse_exp_free(parse_exp_cxt_t *cxt) {
  stack_free(cxt->stacks[0]);
  stack_free(cxt->stacks[1]);
  free(cxt);
  return;
}

// Determine if a token could continue an expression currently being parsed
// Literals (incl. ident), operators, and sizeof() could be part of an expression
int parse_exp_isexp(parse_exp_cxt_t *cxt, token_t *token) {
  token_type_t type = token->type;
  return ((type >= T_OP_BEGIN && type < T_OP_END) || 
          (type >= T_LITERALS_BEGIN && type < T_LITERALS_END) || 
          (type == T_SIZEOF));
}

// Returns whether a token is primary token
int parse_exp_isprimary(parse_exp_cxt_t *cxt, token_t *token) {
  return token->type >= T_LITERALS_BEGIN && token->type < T_LITERALS_END;
}

// Frees the entire token including the literal if it has one
void parse_exp_free_token(token_t *token) {
  token_free_literal(token);
  free(token);
}

// Returned token is allocated from the heap, caller free
// If the token does not belong to expressions, or we reached the end then 
// return NULL
token_t *parse_exp_next_token(parse_exp_cxt_t *cxt) {
  token_t *token = (token_t *)malloc(sizeof(token_t));
  char *before = cxt->s;
  cxt->s = token_get_next(cxt->s, token);
  if(cxt->s == NULL || !parse_exp_isexp(cxt, token)) {
    parse_exp_free_token(token);
    // Reset the text pointer such that the next token is still obtained
    cxt->s = before;
    return NULL;
  }

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
      default: error_row_col_exit(before, 
                                  "Did not expect to see \"%s\" as a postfix operator", 
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
      default: error_row_col_exit(before, 
                                  "Did not expect to see \"%s\" as a prefix operator", 
                                  token_symstr(token->type));
    }
  }

  return token;
}

void parse_exp_shift(parse_exp_cxt_t *cxt, int stack_id, token_t *token) {
  assert(stack_id == OP_STACK || stack_id == AST_STACK);
  stack_push(cxt->stacks[stack_id], token);
  cxt->last_active_stack = stack_id;
  return;
}

// One reduce step of the topmost operator
// Return the next op at stack top; NULL if stack empty
// If op stack is empty then do nothing, and return NULL to indicate this
token_t *parse_exp_reduce(parse_exp_cxt_t *cxt) {
  stack_t *ast = cxt->stacks[AST_STACK], *op = cxt->stacks[OP_STACK];
  if(stack_empty(op)) return NULL;
  token_t *top_op = stack_pop(op);
  int op_num = token_get_num_operand(top_op->type);
  for(int i = 0;i < op_num;i++) {
    token_t *operand = stack_pop(ast);
    // Note that nodes are poped in reverse order
    ast_push_child(top_op, operand);
    if(stack_empty(ast)) {
      error_row_col_exit(cxt->s, "Wrong number of operands for %s\n", token_typestr(top_op->type));
    }
  }

  parse_exp_shift(cxt, AST_STACK, top_op);
  return stack_empty(op) ? NULL : stack_peek(op);
}

void parse_exp(parse_exp_cxt_t *cxt) {
  stack_t *ast = cxt->stacks[AST_STACK], *op = cxt->stacks[OP_STACK];
  while(1) {
    token_t *token = parse_exp_next_token(cxt);
    if(token == NULL) {
      // TODO: FINISHED ALL TOKENS, REDUCE ALL
      // TODO: Maybe check for type cast?
      printf("Reaches the end\n");
      exit(1);
    } else if(parse_exp_isprimary(cxt, token)) {
      ast_make_node(token);
      parse_exp_shift(cxt, AST_STACK, token);
    } else {
      int preced; assoc_t assoc;
      token_get_property(token->type, &preced, &assoc);
      if(stack_empty(op)) {
        stack_push(op, token);
      }
    }
  }
}