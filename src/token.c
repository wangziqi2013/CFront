
#include "token.h"

const char *keywords[32] = {
  "auto", "break", "case", "char", "const", "continue", "default", "do",
  "double", "else", "enum", "extern", "float", "for", "goto", "if",
  "int", "long", "register", "return", "short", "signed", "sizeof", "static",
  "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
};

decl_prop_t kwd_props[32] = {
  DECL_AUTO, // auto
  0, 0,            // break case
  DECL_CHAR, // char
  DECL_CONST_MASK, // const
  0, 0, 0,         // continue default do
  DECL_DOUBLE, // double
  0,               // else
  DECL_ENUM, // enum
  DECL_EXTERN, // extern
  DECL_FLOAT, // float
  0, 0, 0,         // for goto if
  DECL_INT, // int
  DECL_LONG, // long
  DECL_REGISTER, // register
  0,               // return
  DECL_SHORT, // short
  DECL_SIGNED, // signed
  0,               // sizeof
  DECL_STATIC, // static
  DECL_STRUCT, // struct
  0,               // switch
  DECL_TYPEDEF, // typedef
  DECL_UNION, // union
  DECL_UNSIGNED, // unsigned
  DECL_VOID, // void
  DECL_VOLATILE_MASK, // volatile
  0,               // while
};

// The layout of precedences is consistent with the layout of the token table 
// (51 elements); 0 and 999 are not used, which are just placeholders
int precedences[51] = {
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

token_cxt_t *token_cxt_init(char *input) {
  token_cxt_t *cxt = (token_cxt_t *)malloc(sizeof(token_cxt_t));
  if(cxt == NULL) perror(__func__);
  cxt->udef_types = ht_str_init();
  cxt->pushbacks = NULL;
  cxt->s = input;
  cxt->pb_num = 0;
  cxt->ignore_pb = 0;
  return cxt;
}

void token_cxt_free(token_cxt_t *cxt) {
  ht_free(cxt->udef_types);
  if(cxt->pushbacks) {
    token_t *curr = cxt->pushbacks->next;
    cxt->pushbacks->next = NULL;
    while(curr != NULL) {
      cxt->pushbacks = curr->next;
      token_free(curr);
      curr = cxt->pushbacks;
    }
  }
  free(cxt);
}

// Adds a user-defined type
void token_add_utype(token_cxt_t *cxt, token_t *token) {
  ht_insert(cxt->udef_types, token->str, NULL);
}

int token_isutype(token_cxt_t *cxt, token_t *token) {
  return token->type == T_IDENT && ht_find(cxt->udef_types, token->str) != HT_NOTFOUND;
}

// Only checks STORAGE CLASS and QUALIFIER. At most one from the former is allowed.
// No duplicates for the latter is allowed
int token_decl_compatible(token_t *token, decl_prop_t decl_prop) {
  if(token->decl_prop & DECL_STGCLS_MASK) return !(decl_prop & DECL_STGCLS_MASK);
  if(token->decl_prop & DECL_QUAL_MASK) return !(decl_prop & token->decl_prop);
  return 1;
}

// Apply the token's specifier/qualifier/type to the prop and return updated value
// Return DECL_INVALID if incompatible
int token_decl_apply(token_t *token, decl_prop_t decl_prop) {
  if(!token_decl_compatible(token, decl_prop)) return 0;
  token->decl_prop |= decl_prop;
  return 1;
}

// Returns a buffer that contains string representation of the property bit mask
char *token_decl_print(decl_prop_t decl_prop) {
  static char buffer[512]; buffer[0] = '\0';  // Should be sufficient?
  if(decl_prop & DECL_STGCLS_MASK) {
    switch(decl_prop & DECL_STGCLS_MASK) {
      case DECL_TYPEDEF: strcat(buffer, "typedef "); break;
      case DECL_EXTERN: strcat(buffer, "extern "); break;
      case DECL_AUTO: strcat(buffer, "auto "); break;
      case DECL_REGISTER: strcat(buffer, "register "); break;
      case DECL_STATIC: strcat(buffer, "static "); break;
    }
  }
  if(decl_prop & DECL_VOLATILE_MASK) strcat(buffer, "volatile ");
  if(decl_prop & DECL_CONST_MASK) strcat(buffer, "const ");
  if(decl_prop & DECL_SIGN_MASK) {
    switch(decl_prop & DECL_SIGN_MASK) {
      case DECL_SIGNED: strcat(buffer, "signed "); break;
      case DECL_UNSIGNED: strcat(buffer, "unsigned "); break;
    }
  }
  if(decl_prop & DECL_TYPE_MASK) {
    switch(decl_prop & DECL_TYPE_MASK) {
      case DECL_CHAR: strcat(buffer, "char "); break;
      case DECL_SHORT: strcat(buffer, "short "); break;
      case DECL_INT: strcat(buffer, "int "); break;
      case DECL_LONG: strcat(buffer, "long "); break;
      case DECL_ENUM: strcat(buffer, "enum "); break;
      case DECL_STRUCT: strcat(buffer, "struct "); break;
      case DECL_UNION: strcat(buffer, "union "); break;
      case DECL_UDEF: strcat(buffer, "<typedef'ed> "); break;
      case DECL_FLOAT: strcat(buffer, "float "); break;
      case DECL_DOUBLE: strcat(buffer, "double "); break;
      case DECL_VOID: strcat(buffer, "void "); break;
    }
  }
  return buffer;
}

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

// Number of operands of the token. Most operands below precedence 2 has two
// operands, the only exception being : ? which has three.
int token_get_num_operand(token_type_t type) {
  assert(type >= EXP_BEGIN && type < EXP_END);
  // Only case in precedence 1 and has 1 operand
  if(type == EXP_DOT || type == EXP_ARROW || 
     type == EXP_ARRAY_SUB || type == EXP_FUNC_CALL) return 2;
  int preced = precedences[type - EXP_BEGIN];
  assert(preced != 0 && preced != 999);
  if(preced > 2)
    if(preced == 13) return 2;
    else return 2;
  else return 1;
}

// Return T_ILLEGAL if not a keyword; keyword type otherwise
// Note:
//   1. Argument s must be a null-terminated string
token_type_t token_get_keyword_type(const char *s) {
  int begin = 0, end = sizeof(keywords) / sizeof(const char *);
  while(begin < end - 1) {
    int middle = (begin + end) / 2;
    int cmp = strcmp(keywords[middle], s);
    if(cmp == 0) return T_KEYWORDS_BEGIN + middle;
    else if(cmp < 0) begin = middle + 1;
    else end = middle;
    assert(begin < (int)sizeof(keywords) / (int)sizeof(const char *));
    assert(end <= (int)sizeof(keywords) / (int)sizeof(const char *));
  }
  // This means the given token is not a keyword
  if(strcmp(keywords[begin], s) == 0) return T_KEYWORDS_BEGIN + begin;
  return T_ILLEGAL;
}

// Converts the token type to a string
// Note that all types except the illegal has a string representation
const char *token_typestr(token_type_t type) {
  switch(type) {
    case T_LPAREN: return "T_LPAREN";
    case T_RPAREN: return "T_RPAREN";
    case T_LSPAREN: return "T_LSPAREN";
    case T_RSPAREN: return "T_RSPAREN";
    case T_DOT: return "T_DOT";
    case T_ARROW: return "T_ARROW";
    case T_INC: return "T_INC";
    case T_DEC: return "T_DEC";
    case T_PLUS: return "T_PLUS";
    case T_MINUS: return "T_MINUS";
    case T_LOGICAL_NOT: return "T_LOGICAL_NOT";
    case T_BIT_NOT: return "T_BIT_NOT";
    case T_STAR: return "T_STAR";
    case T_AND: return "T_AND";
    //case T_SIZEOF: return "T_SIZEOF";
    case T_DIV: return "T_DIV";
    case T_MOD: return "T_MOD";
    case T_LSHIFT: return "T_LSHIFT";
    case T_RSHIFT: return "T_RSHIFT";
    case T_LESS: return "T_LESS";
    case T_GREATER: return "T_GREATER";
    case T_LEQ: return "T_LEQ";
    case T_GEQ: return "T_GEQ";
    case T_EQ: return "T_EQ";
    case T_NEQ: return "T_NEQ";
    case T_BIT_XOR: return "T_BIT_XOR";
    case T_BIT_OR: return "T_BIT_OR";
    case T_LOGICAL_AND: return "T_LOGICAL_AND";
    case T_LOGICAL_OR: return "T_LOGICAL_OR";
    case T_QMARK: return "T_QMARK";
    case T_COLON: return "T_COLON";
    case T_ASSIGN: return "T_ASSIGN";
    case T_PLUS_ASSIGN: return "T_PLUS_ASSIGN";
    case T_MINUS_ASSIGN: return "T_MINUS_ASSIGN";
    case T_MUL_ASSIGN: return "T_MUL_ASSIGN";
    case T_DIV_ASSIGN: return "T_DIV_ASSIGN";
    case T_MOD_ASSIGN: return "T_MOD_ASSIGN";
    case T_LSHIFT_ASSIGN: return "T_LSHIFT_ASSIGN";
    case T_RSHIFT_ASSIGN: return "T_RSHIFT_ASSIGN";
    case T_AND_ASSIGN: return "T_AND_ASSIGN";
    case T_OR_ASSIGN: return "T_OR_ASSIGN";
    case T_XOR_ASSIGN: return "T_XOR_ASSIGN";
    case T_COMMA: return "T_COMMA";
    case T_LCPAREN: return "T_LCPAREN";
    case T_RCPAREN: return "T_RCPAREN";
    case T_SEMICOLON: return "T_SEMICOLON";
    case T_ELLIPSIS: return "T_ELLIPSIS";
    // Literal types
    case T_DEC_INT_CONST: return "T_DEC_INT_CONST";
    case T_HEX_INT_CONST: return "T_HEX_INT_CONST";
    case T_OCT_INT_CONST: return "T_OCT_INT_CONST";
    case T_CHAR_CONST: return "T_CHAR_CONST";
    case T_STR_CONST: return "T_STR_CONST";
    case T_FLOAT_CONST: return "T_FLOAT_CONST";
    // User defined identifier
    case T_IDENT: return "T_IDENT";
    // Keywords
    case T_AUTO: return "T_AUTO"; 
    case T_BREAK: return "T_BREAK"; 
    case T_CASE: return "T_CASE"; 
    case T_CHAR: return "T_CHAR"; 
    case T_CONST: return "T_CONST"; 
    case T_CONTINUE: return "T_CONTINUE"; 
    case T_DEFAULT: return "T_DEFAULT"; 
    case T_DO: return "T_DO";
    case T_DOUBLE: return "T_DOUBLE"; 
    case T_ELSE: return "T_ELSE";
    case T_ENUM: return "T_ENUM"; 
    case T_EXTERN: return "T_EXTERN";
    case T_FLOAT: return "T_FLOAT";
    case T_FOR: return "T_FOR";
    case T_GOTO: return "T_GOTO"; 
    case T_IF: return "T_IF";
    case T_INT: return "T_INT";
    case T_LONG: return "T_LONG"; 
    case T_REGISTER: return "T_REGISTER";
    case T_RETURN: return "T_RETURN"; 
    case T_SHORT: return "T_SHORT";
    case T_SIGNED: return "T_SIGNED";
    case T_SIZEOF: return "T_SIZEOF";
    case T_STATIC: return "T_STATIC";
    case T_STRUCT: return "T_STRUCT";
    case T_SWITCH: return "T_SWITCH";
    case T_TYPEDEF: return "T_TYPEDEF";
    case T_UNION: return "T_UNION";
    case T_UNSIGNED: return "T_UNSIGNED";
    case T_VOID: return "T_VOID";
    case T_VOLATILE: return "T_VOLATILE";
    case T_WHILE: return "T_WHILE";
    // Expressions
    case EXP_FUNC_CALL: return "EXP_FUNC_CALL";
    case EXP_ARRAY_SUB: return "EXP_ARRAY_SUB";
    case EXP_LPAREN: return "EXP_LPAREN";
    case EXP_RPAREN: return "EXP_RPAREN";
    case EXP_RSPAREN: return "EXP_RSPAREN";
    case EXP_DOT: return "EXP_DOT";
    case EXP_ARROW: return "EXP_ARROW";
    case EXP_POST_INC: return "EXP_POST_INC";
    case EXP_PRE_INC: return "EXP_PRE_INC";
    case EXP_POST_DEC: return "EXP_POST_DEC";
    case EXP_PRE_DEC: return "EXP_PRE_DEC";
    case EXP_PLUS: return "EXP_PLUS";
    case EXP_MINUS: return "EXP_MINUS";
    case EXP_LOGICAL_NOT: return "EXP_LOGICAL_NOT";
    case EXP_BIT_NOT: return "EXP_BIT_NOT";
    case EXP_CAST: return "EXP_CAST";
    case EXP_DEREF: return "EXP_DEREF";
    case EXP_ADDR: return "EXP_ADDR";
    case EXP_SIZEOF: return "EXP_SIZEOF";
    case EXP_MUL: return "EXP_MUL";
    case EXP_DIV: return "EXP_DIV";
    case EXP_MOD: return "EXP_MOD";
    case EXP_ADD: return "EXP_ADD";
    case EXP_SUB: return "EXP_SUB";
    case EXP_LSHIFT: return "EXP_LSHIFT";
    case EXP_RSHIFT: return "EXP_RSHIFT";
    case EXP_LESS: return "EXP_LESS";
    case EXP_GREATER: return "EXP_GREATER";
    case EXP_LEQ: return "EXP_LEQ";
    case EXP_GEQ: return "EXP_GEQ";
    case EXP_EQ: return "EXP_EQ";
    case EXP_NEQ: return "EXP_NEQ";
    case EXP_BIT_AND: return "EXP_BIT_AND";
    case EXP_BIT_OR: return "EXP_BIT_OR";
    case EXP_BIT_XOR: return "EXP_BIT_XOR";
    case EXP_LOGICAL_AND: return "EXP_LOGICAL_AND";
    case EXP_LOGICAL_OR: return "EXP_LOGICAL_OR";
    case EXP_COND: return "EXP_COND";
    case EXP_COLON: return "EXP_COLON";
    case EXP_ASSIGN: return "EXP_ASSIGN";
    case EXP_ADD_ASSIGN: return "EXP_ADD_ASSIGN";
    case EXP_SUB_ASSIGN: return "EXP_SUB_ASSIGN";
    case EXP_MUL_ASSIGN: return "EXP_MUL_ASSIGN";
    case EXP_DIV_ASSIGN: return "EXP_DIV_ASSIGN";
    case EXP_MOD_ASSIGN: return "EXP_MOD_ASSIGN";
    case EXP_AND_ASSIGN: return "EXP_AND_ASSIGN";
    case EXP_OR_ASSIGN: return "EXP_OR_ASSIGN";
    case EXP_XOR_ASSIGN: return "EXP_XOR_ASSIGN";
    case EXP_LSHIFT_ASSIGN: return "EXP_LSHIFT_ASSIGN";
    case EXP_RSHIFT_ASSIGN: return "EXP_RSHIFT_ASSIGN";
    case EXP_COMMA: return "EXP_COMMA";
    case T_UDEF: return "T_UDEF";
    case T_DECL: return "T_DECL"; 
    case T_BASETYPE: return "T_BASETYPE";
    case T_: return "T_";
    case T_COMP_DECL: return "T_COMP_DECL"; 
    case T_COMP_FIELD: return "T_COMP_FIELD";
    case T_ILLEGAL: return "T_ILLEGAL";
    default: return "(unknown)";
  }

  return NULL;
}

// Return the actual symbol of the token type
// For identifier and literal types and non-token types (e.g. expressions) this 
// function returns NULL
const char *token_symstr(token_type_t type) {
  assert(type != T_ILLEGAL);
  switch(type) {
    case T_LPAREN: return "(";
    case T_RPAREN: return ")";
    case T_LSPAREN: return "[";
    case T_RSPAREN: return "]";
    case T_DOT: return ".";
    case T_ARROW: return "->";
    case T_INC: return "++";
    case T_DEC: return "--";
    case T_PLUS: return "+";
    case T_MINUS: return "-";
    case T_LOGICAL_NOT: return "!";
    case T_BIT_NOT: return "~";
    case T_STAR: return "*";
    case T_AND: return "&";
    //case T_SIZEOF: return "sizeof";
    case T_DIV: return "/";
    case T_MOD: return "%";
    case T_LSHIFT: return "<<";
    case T_RSHIFT: return ">>";
    case T_LESS: return "<";
    case T_GREATER: return ">";
    case T_LEQ: return "<=";
    case T_GEQ: return ">=";
    case T_EQ: return "==";
    case T_NEQ: return "!=";
    case T_BIT_XOR: return "^";
    case T_BIT_OR: return "|";
    case T_LOGICAL_AND: return "&&";
    case T_LOGICAL_OR: return "||";
    case T_QMARK: return "?";
    case T_COLON: return ":";
    case T_ASSIGN: return "=";
    case T_PLUS_ASSIGN: return "+=";
    case T_MINUS_ASSIGN: return "-=";
    case T_MUL_ASSIGN: return "*=";
    case T_DIV_ASSIGN: return "/+";
    case T_MOD_ASSIGN: return "%=";
    case T_LSHIFT_ASSIGN: return "<<=";
    case T_RSHIFT_ASSIGN: return ">>=";
    case T_AND_ASSIGN: return "&=";
    case T_OR_ASSIGN: return "|=";
    case T_XOR_ASSIGN: return "^=";
    case T_COMMA: return ",";
    case T_LCPAREN: return "{";
    case T_RCPAREN: return "}";
    case T_SEMICOLON: return ";";
    case T_ELLIPSIS: return "...";
    // keywords
    case T_AUTO: return "auto"; 
    case T_BREAK: return "break"; 
    case T_CASE: return "case"; 
    case T_CHAR: return "char"; 
    case T_CONST: return "const"; 
    case T_CONTINUE: return "continue"; 
    case T_DEFAULT: return "default"; 
    case T_DO: return "do";
    case T_DOUBLE: return "double"; 
    case T_ELSE: return "else";
    case T_ENUM: return "enum"; 
    case T_EXTERN: return "extern";
    case T_FLOAT: return "float";
    case T_FOR: return "for";
    case T_GOTO: return "goto"; 
    case T_IF: return "if";
    case T_INT: return "int";
    case T_LONG: return "long"; 
    case T_REGISTER: return "register";
    case T_RETURN: return "return"; 
    case T_SHORT: return "short";
    case T_SIGNED: return "signed";
    case T_SIZEOF: return "sizeof";
    case T_STATIC: return "static";
    case T_STRUCT: return "struct";
    case T_SWITCH: return "switch";
    case T_TYPEDEF: return "typedef";
    case T_UNION: return "union";
    case T_UNSIGNED: return "unsigned";
    case T_VOID: return "void";
    case T_VOLATILE: return "volatile";
    case T_WHILE: return "while";
    default: return NULL;
  }
}

// Fill an operator token object according to its type
// Return value:
//   1. If input is not '\0' then return the next unread character
//   2. If input is '\0' then return NULL
//   3. If not valid operator could be found then token type is T_ILLEGAL
//      and the pointer is not changed
// Note: 
//   1. sizeof() is treated as a keyword by the tokenizer
//   2. // and /* and */ and // are not processed
//   3. { and } are processed here
char *token_get_op(char *s, token_t *token) {
  token->offset = s;
  if(s == NULL) return NULL;
  switch(s[0]) {
    case '\0': return NULL;
    // Must be single character operator
    case ';': token->type = T_SEMICOLON; return s + 1;                  // Not really an operator
    case ',': token->type = T_COMMA; return s + 1;                      // ,
    case '(': token->type = T_LPAREN; return s + 1;                     // (
    case ')': token->type = T_RPAREN; return s + 1;                     // )
    case '[': token->type = T_LSPAREN; return s + 1;                    // [
    case ']': token->type = T_RSPAREN; return s + 1;                    // ]
    case '{': token->type = T_LCPAREN; return s + 1;                    // {
    case '}': token->type = T_RCPAREN; return s + 1;                    // }
    case '.': 
      switch(s[1]) {
        case '.': 
          switch(s[2]) {
            case '.': token->type = T_ELLIPSIS; return s + 3;
            case '\0':
            default: token->type = T_DOT; return s + 1;
          }
        case '\0':
        default: token->type = T_DOT; return s + 1;                     // .
      }
    case '?': token->type = T_QMARK; return s + 1;                      // ?
    case ':': token->type = T_COLON; return s + 1;                      // :
    case '~': token->type = T_BIT_NOT; return s + 1;                    // ~
    // Multi character
    case '-':
      switch(s[1]) {
        case '-': token->type = T_DEC; return s + 2;                    // --
        case '=': token->type = T_MINUS_ASSIGN; return s + 2;           // -=
        case '>': token->type = T_ARROW; return s + 2;                  // ->
        case '\0': 
        default: token->type = T_MINUS; return s + 1;                   // -
      }
    case '+':
      switch(s[1]) {
        case '+': token->type = T_INC; return s + 2;                    // ++
        case '=': token->type = T_PLUS_ASSIGN; return s + 2;            // +=
        case '\0': 
        default: token->type = T_PLUS; return s + 1;                    // +
      }
    case '*':
      switch(s[1]) {
        case '=': token->type = T_MUL_ASSIGN; return s + 2;             // *=
        case '\0': 
        default: token->type = T_STAR; return s + 1;                    // *
      }
    case '/':
      switch(s[1]) {
        case '=': token->type = T_DIV_ASSIGN; return s + 2;             // /=
        case '\0': 
        default: token->type = T_DIV; return s + 1;                     // /
      }
    case '%':
      switch(s[1]) {
        case '=': token->type = T_MOD_ASSIGN; return s + 2;             // %=
        case '\0': 
        default: token->type = T_MOD; return s + 1;                     // %
      }
    case '^':
      switch(s[1]) {
        case '=': token->type = T_XOR_ASSIGN; return s + 2;             // ^=
        case '\0': 
        default: token->type = T_BIT_XOR; return s + 1;                 // ^
      }
    case '<':
      switch(s[1]) {
        case '=': token->type = T_LEQ; return s + 2;                    // <=
        case '<': 
          switch(s[2]) {
            case '=': token->type = T_LSHIFT_ASSIGN; return s + 3;      // <<=
            case '\0':
            default: token->type = T_LSHIFT; return s + 2;              // <<
          } 
        case '\0': 
        default: token->type = T_LESS; return s + 1;                    // <
      }
    case '>':
      switch(s[1]) {
        case '=': token->type = T_GEQ; return s + 2;                    // >=
        case '>': 
          switch(s[2]) {
            case '=': token->type = T_RSHIFT_ASSIGN; return s + 3;      // >>=
            case '\0':
            default: token->type = T_RSHIFT; return s + 2;              // >>
          } 
        case '\0': 
        default: token->type = T_GREATER; return s + 1;                 // >
      }
    case '=':
      switch(s[1]) {
        case '=': token->type = T_EQ; return s + 2;                     // ==
        case '\0': 
        default: token->type = T_ASSIGN; return s + 1;                  // =
      }
    case '!':
      switch(s[1]) {
        case '=': token->type = T_NEQ; return s + 2;                     // !=
        case '\0': 
        default: token->type = T_LOGICAL_NOT; return s + 1;              // !
      }
    case '&':
      switch(s[1]) {
        case '&': token->type = T_LOGICAL_AND; return s + 2;             // &&
        case '=': token->type = T_AND_ASSIGN; return s + 2;              // &=
        case '\0': 
        default: token->type = T_AND; return s + 1;                      // &
      }
    case '|':
      switch(s[1]) {
        case '|': token->type = T_LOGICAL_OR; return s + 2;             // ||
        case '=': token->type = T_OR_ASSIGN; return s + 2;              // |=
        case '\0': 
        default: token->type = T_BIT_OR; return s + 1;                  // |
      }
  }

  token->type = T_ILLEGAL;
  return s;
}

// Copies ident, int, char, str, etc. literal into the token
// Argument end is the first character after the literal
void token_copy_literal(token_t *token, const char *begin, const char *end) {
  token->str = malloc(sizeof(char) * (end - begin + 1));
  if(token->str == NULL) perror(__func__);
  memcpy(token->str, begin, end - begin);
  token->str[end - begin] = '\0';
  return;
}

// Frees the literal buffer in the token. If not a literal then do nothing
void token_free_literal(token_t *token) {
  if(token->type >= T_LITERALS_BEGIN && token->type < T_LITERALS_END) free(token->str);
}

void token_free(token_t *token) {
  token_free_literal(token);
  free(token);
}

token_t *token_alloc() {
  token_t *token = (token_t *)malloc(sizeof(token_t));
  if(token == NULL) perror(__func__);
  token->child = token->sibling = NULL;
  token->str = NULL;
  token->type = T_ILLEGAL;
  token->offset = NULL;
  token->decl_prop = DECL_NULL;
  return token;
}

token_t *token_alloc_type(token_type_t type) {
  token_t *token = token_alloc();
  token->type = type;
  return token;
}

token_t *token_get_empty() { return token_alloc_type(T_); }

// Returns an identifier, including both keywords and user defined identifier
// Same rule as the get_op call
// Note:
//   1. If keywords are detected then the buffer is not initialized
char *token_get_ident(token_cxt_t *cxt, char *s, token_t *token) {
  token->offset = s;
  if(s == NULL || *s == '\0') return NULL;
  else if(isalpha(*s) || *s == '_') {
    char *end = s + 1;
    while(isalnum(*end) || *end == '_') end++;
    // Exchange end with '\0' in order to call the function
    char temp = *end; *end = '\0';
    token_type_t type = token_get_keyword_type(s);
    *end = temp;
    if(type == T_ILLEGAL) {
      token->type = T_IDENT;
      token_copy_literal(token, s, end);
      if(token_isutype(cxt, token)) {
        token->type = T_UDEF;
        token->decl_prop |= DECL_UDEF;
      }
    } else {
      token->type = type;
      assert(type >= T_KEYWORDS_BEGIN && type < T_KEYWORDS_END);
      token->decl_prop = kwd_props[type - T_KEYWORDS_BEGIN];
    }
    return end;
  }
  
  token->type = T_ILLEGAL;
  return s;
}

char *token_get_int(char *s, token_t *token) {
  token->offset = s;
  if(s == NULL || *s == '\0') return NULL;
  token->type = T_DEC_INT_CONST;
  if(s[0] == '0') {
    if(s[1] == 'x') {
      if(isxdigit(s[2])) {
        s += 2;
        token->type = T_HEX_INT_CONST;
      } else error_exit("Invalid hex integer literal\n");
    } else if(isdigit(s[1])) {
      s++;
      token->type = T_OCT_INT_CONST;
    }
  }
  char *end = s;
  if(token->type == T_DEC_INT_CONST) while(isdigit(*end)) end++;
  else if(token->type == T_HEX_INT_CONST) while(isxdigit(*end)) end++;
  else while(*end >= '0' && *end < '8') end++;
  assert(end != s);
  token_copy_literal(token, s, end);

  return end;
}

char *token_get_str(char *s, token_t *token, char closing) {
  token->offset = s;
  if(s == NULL || *s == '\0') return NULL;
  if(closing == '\'') token->type = T_CHAR_CONST;
  else token->type = T_STR_CONST;
  char *end = s;
  do {
    while(*end != '\0' && *end != closing) end++;
    if(*end == '\0') error_exit("%s literal not closed\n", closing == '\"' ? "String" : "Char");
  } while(end[-1] == '\\');
  token_copy_literal(token, s, end);

  return end + 1;
}

// Returns the next token, or illegal
// token must be allocated on the heap, not stack
token_t *token_get_next(token_cxt_t *cxt) {
  token_t *token;
  if(cxt->pushbacks && !cxt->ignore_pb) {
    assert(cxt->pb_num > 0);
    token = cxt->pushbacks->next;
    if(token == cxt->pushbacks) cxt->pushbacks = NULL;
    else cxt->pushbacks->next = token->next;
    cxt->pb_num--;
    return token;
  }
  token = token_alloc();
  while(1) {
    const char *before = cxt->s;
    if(cxt->s == NULL || *cxt->s == '\0') { token_free(token); return NULL; }
    else if(isspace(*cxt->s)) while(isspace(*cxt->s)) cxt->s++;
    else if(cxt->s[0] == '/' && cxt->s[1] == '/') while(*cxt->s != '\n' && *cxt->s != '\0') cxt->s++;
    else if(cxt->s[0] == '/' && cxt->s[1] == '*') {
      cxt->s += 2;
      while((cxt->s[0] != '\0') && (cxt->s[0] != '*' || cxt->s[1] != '/')) cxt->s++;
      if(cxt->s[0] == '\0') error_row_col_exit(before, "Block comment not closed at the end of file\n");
      cxt->s += 2;
    }
    else if(isalpha(*cxt->s) || *cxt->s == '_') { cxt->s = token_get_ident(cxt, cxt->s, token); break; }
    else if(isdigit(*cxt->s))                   { cxt->s = token_get_int(cxt->s, token); break; }
    else if(*cxt->s == '\'' || *cxt->s == '\"') { cxt->s = token_get_str(cxt->s + 1, token, *cxt->s); break; }
    else {
      cxt->s = token_get_op(cxt->s, token);
      if(token->type == T_ILLEGAL) error_row_col_exit(before, "Unknown symbol \'%c\'\n", *before);
      break;
    }
  }
  return token;
}

// Consume the next token if it is of the given type. 
// Return 0 if there is no next token or type mismatch, and the token is not consumed
int token_consume_type(token_cxt_t *cxt, token_type_t type) {
  token_t *token = token_get_next(cxt);
  if(token == NULL) return 0;
  if(token->type == type) { token_free(token); return 1; }
  token_pushback(cxt, token);
  return 0;
}

void token_pushback(token_cxt_t *cxt, token_t *token) {
  assert(token != NULL);
  if(cxt->pushbacks == NULL) cxt->pushbacks = token->next = token;
  else {
    token->next = cxt->pushbacks->next;
    cxt->pushbacks->next = token;
    assert(token->next != NULL);
    cxt->pushbacks = token;
  }
  cxt->pb_num++;
}

// Looks ahead into the token stream. If token stream ended before num then return NULL
// Return value cannot be used
token_t *token_lookahead(token_cxt_t *cxt, int num) {
  assert(num > 0 && cxt->pb_num >= 0);  
  while(cxt->pb_num < num) {
    cxt->ignore_pb = 1;
    token_t *token = token_get_next(cxt); // This may return NULL if token stream reaches the end
    cxt->ignore_pb = 0;
    if(token != NULL) token_pushback(cxt, token);
    else return NULL;
  }
  token_t *ret = cxt->pushbacks;
  if(cxt->pb_num != num) while(num--) ret = ret->next;
  return ret;
}

// Same as the regular version except that it reports error if run out of tokens
token_t *token_lookahead_notnull(token_cxt_t *cxt, int num) {
  token_t *la = token_lookahead(cxt, num);
  if(la == NULL) error_row_col_exit(cxt->s, "Unexpected end of file\n");
  return la;
}