
#include "parse_exp.h"

// The layout of precedences is consistent with the layout of the token table 
// (51 elements)
int precedences[] = {
  1, 1,       // EXP_FUNC_CALL, EXP_ARRAY_SUB, f() arr[]
  0,          // EXP_LPAREN, (exp...
  999, 999,   // EXP_RPAREN, EXP_RSPAREN, ) ] <-- Force reduce of all operators
  1, 1,       // EXP_DOT, EXP_ARROW, obj.field obj->field
  1, 2, 1, 2, // EXP_POST_INC, EXP_PRE_INC, EXP_POST_DEC, EXP_PRE_DEC
  2, 2,       // EXP_PLUS, EXP_MINUS, +a -a
  2, 2,       // EXP_LOGICAL_NOT, EXP_BIT_NOT, !exp ~exp
  2,          // EXP_CAST, (type)
  2, 2,       // EXP_DEREF, EXP_ADDR, *ptr &x
  2,          // EXP_SIZEOF,                 // sizeof(type/name)
  3, 3, 3,    // EXP_MUL, EXP_DIV, EXP_MOD,  // binary * / %
  4, 4,       // EXP_ADD, EXP_SUB,           // binary + -
  5, 5,       // EXP_LSHIFT, EXP_RSHIFT,     // << >>
  6, 6, 6, 6, // EXP_LESS, EXP_GREATER, EXP_LEQ, EXP_GEQ, // < > <= >=
  7, 7,       // EXP_EQ, EXP_NEQ,            // == !=
  8, 9, 10,   // EXP_BIT_AND, EXP_BIT_OR, EXP_BIT_XOR,    // binary & | ^
  11, 12,     // EXP_LOGICAL_AND, EXP_LOGICAL_OR,         // && ||
  13, 13,     // EXP_COND, EXP_COLON, ? :
  14, 14, 14, // EXP_ASSIGN, EXP_ADD_ASSIGN, EXP_SUB_ASSIGN,          // = += -=
  14, 14, 14, // EXP_MUL_ASSIGN, EXP_DIV_ASSIGN, EXP_MOD_ASSIGN, // *= /= %=
  14, 14, 14, // EXP_AND_ASSIGN, EXP_OR_ASSIGN, EXP_XOR_ASSIGN,  // &= |= ^=
  14, 14,     // EXP_LSHIFT_ASSIGN, EXP_RSHIFT_ASSIGN,    // <<= >>=
  15,         // EXP_COMMA,                               // binary ,
};

// Returns the precedence and associativity
// Associativity is encoded implicitly by precedence: 2, 13 and 14 are R-TO-L
// and the rest are L-TO-R 
void token_get_property(token_type_t type, int *preced, assoc_t *assoc) {
  assert(type >= EXP_BEGIN && type < EXP_END);
  assert(sizeof(precedences) / sizeof(precedences[0]) == (EXP_END - EXP_BEGIN));
  *preced = precedences[type - EXP_BEGIN];
  if(*preced == 2 || *preced == 13 || *preced == 14) *assoc = ASSOC_RL;
  else *assoc = ASSOC_LR;
  return;
}

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
  if(cxt->allow_comma == 0 && type == T_COMMA) return 0; 
  return ((type >= T_OP_BEGIN && type < T_OP_END) || 
          (type >= T_LITERALS_BEGIN && type < T_LITERALS_END) || 
          (type == T_SIZEOF));
}

// Returned token is allocated from the heap, caller free
// If the token does not belong to expressions, or we reached the end then 
// return NULL
token_t *parse_exp_next_token(parse_exp_cxt_t *cxt) {
  token_t *token = (token_t *)malloc(sizeof(token_t));
  char *before = cxt->s;
  cxt->s = token_get_next(cxt->s, token);
  if(cxt->s == NULL || !parse_exp_isexp(cxt, token)) {
    free(token);
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

void parse_exp(parse_exp_cxt_t *cxt) {
  while(1) {
    token_t *token = parse_exp_next_token(cxt);
    if(token == NULL) {
      // TODO: FINISHED ALL TOKENS
      // TODO: Maybe check for type cast?
      printf("Reaches the end\n");
      exit(1);
    }
  }
}