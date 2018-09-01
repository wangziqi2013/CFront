
#include "token.h"

const char *keywords[] = {
  "auto", "break", "case", "char", "const", "continue", "default", "do",
  "double", "else", "enum", "extern", "float", "for", "goto", "if",
  "int", "long", "register", "return", "short", "signed", "sizeof", "static",
  "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
};

// Return T_ILLEGAL if not a keyword; keyword type otherwise
// Note:
//   1. Argument s must be a null-terminated string
token_type_t get_keyword_type(const char *s) {
  int begin = 0, end = sizeof(keywords) / sizeof(const char *);
  while(begin < end - 1) {
    int middle = (begin + end) / 2;
    int cmp = strcmp(keywords[middle], s);
    if(cmp == 0) return T_KEYWORDS + middle;
    else if(cmp < 0) begin = middle + 1;
    else end = middle;
    assert(begin < sizeof(keywords) / sizeof(const char *));
    assert(end <= sizeof(keywords) / sizeof(const char *));
  }
  // This means the given token is not a keyword
  if(strcmp(keywords[begin], s) == 0) return T_KEYWORDS + begin;
  return T_ILLEGAL;
}

// Converts the token type to a string
const char *token_typestr(token_type_t type) {
  assert(type != T_ILLEGAL);
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
  }

  assert(0);
  return NULL;
}

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
  }
  
  //printf("%d\n", type);
  assert(0);
  return NULL;
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
  if(s == NULL) return NULL;
  switch(s[0]) {
    case '\0': return NULL;
    // Must be single character operator
    case ',': token->type = T_COMMA; return s + 1;                      // ,
    case '(': token->type = T_LPAREN; return s + 1;                    // (
    case ')': token->type = T_RPAREN; return s + 1;                    // )
    case '[': token->type = T_LSPAREN; return s + 1;                    // [
    case ']': token->type = T_RSPAREN; return s + 1;                    // ]
    case '{': token->type = T_LCPAREN; return s + 1;                    // {
    case '}': token->type = T_RCPAREN; return s + 1;                    // }
    case '.': token->type = T_DOT; return s + 1;                        // .
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

// Returns an identifier, including both keywords and user defined identifier
// Same rule as the get_op call
// Note:
//   1. If keywords are detected then the string will be NULL
char *token_get_ident(char *s, token_t *token) {
  if(s == NULL) return NULL;
  char ch = *s;
  if(ch == '\0') {
    return NULL;
  } else if(isalpha(ch) || ch == '_') {
    char *end = s + 1;
    while(isalnum(*end) || *end == '_') end++;
    // Exchange end with '\0' in order to call the function
    char temp = *end; *end = '\0';
    token_type_t type = get_keyword_type(s);
    if(type == T_ILLEGAL) {
      token->str = (char *)malloc(sizeof(char) * (end - s + 1));
      if(token->str == NULL) perror(__func__);
      strcpy(token->str, s);
      token->type = T_IDENT;
    } else {
      token->type = type;
    }
    // Always restore the char
    *end = temp;
    return end;
  }
  
  token->type = T_ILLEGAL;
  return s;
}

char *token_get_int(char *s, token_t *token) {
  if(s == NULL || *s == '\0') return NULL;
  token->type = T_DEC_INT_CONST;
  if(s[0] == '0') {
    if(s[1] == 'x') {
      if(isxdigit(s[2])) {
        s += 2;
        token->type = T_HEX_INT_CONST;
      } else fprintf(stderr, "Invalid hex integer literal\n");
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

  token->str = malloc(sizeof(char) * (end - s + 1));
  if(token->str == NULL) perror(__func__);
  memcpy(token->str, s, end - s);
  token->str[end - s] = '\0';

  return end;
}

// Returns the next token, or illegal
// Same rule for return value and conditions as token_get_op()
char *token_get_next(char *s, token_t *token) {
  while(1) {
    if(s == NULL || *s == '\0') return NULL;
    else if(isspace(*s)) while(isspace(*s)) s++;
    else if(s[0] == '/' && s[1] == '/') while(*s != '\n' && *s != '\0') s++;
    else if(s[0] == '/' && s[1] == '*') {
      while((s[0] != '\0') && (s[0] != '*' || s[1] != '/')) s++;
      s += 2;
    }
    else if(isalpha(*s) || *s == '_') return token_get_ident(s, token);
    else if(isdigit(*s)) return token_get_int(s, token);
    else return token_get_op(s, token);
  }

  assert(0);
  return NULL;
}