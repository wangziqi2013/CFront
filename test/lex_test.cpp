
#include "test_suite.h"

/*
 * TestLexClipCharLiteral() - Tests whether chat literal of all escaped
 *                            and non-escaped form could be cliped correctly
 */
void TestLexClipCharLiteral() {
  // The last should err
  char data1[] = "'d''\\n''1''\\'''\\x13''\\67''\\\\''\\\"''\\x11234";
  char data2[] = {'d', '\n', '1', '\'', '\x13', '\67', '\\', '\"', ' '};
  
  printf("Test string: %s\n", data1);
  
  SourceFile sf{data1, sizeof(data1) - 1};
  
  for(int i = 0;i < sizeof(data2);i++) {
    try {
      char ch = sf.ClipCharLiteral();
      
      printf("Char Literal = %d\n", ch);
      
      assert(ch == data2[i]);
    } catch(const std::string &reason) {
      std::cout << reason << std::endl;
      
      return;
    }
  }
  
  return;
}

/*
 * TestLexClipStringLiteral() - Tests whether the lex clips a string literal
 *                              correctly
 */
void TestLexClipStringLiteral() {
  char data[] = "\"This is a string \naaa\tbbb\t\x0anew line\"\"This is second \n string\"";
  
  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  try {
    auto ret = sf.ClipStringLiteral();
    auto ret2 = sf.ClipStringLiteral();
    printf("String Literal = %s\n", ret->c_str());
    printf("String Literal = %s\n", ret2->c_str());
    
    assert((AppendDoubleQuote(*ret) +
            AppendDoubleQuote(*ret2)) == std::string{data});
    
    
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}

/*
 * TestClipIntegerLiteral() - Clips an integer literal
 */
void TestLexClipIntegerLiteral() {
  char data[] = "12345 \n\n 0x9876 \t\t 0777   000   0  0x23 \n\t 0888";
  unsigned long data2[] = {12345, 0x9876, 0777, 000, 0, 0x23, 0};
  
  SourceFile sf{data, sizeof(data) - 1};
  
  for(int i = 0;i < sizeof(data2) / sizeof(unsigned long);i++) {
    unsigned long v;
    
    try {
      sf.SkipSpace();

      v = sf.ClipIntegerLiteral();
    } catch(const std::string &reason) {
      std::cout << reason << std::endl;

      return;
    }
    
    printf("Value = %lu\n", v);
    
    assert(v == data2[i]);
  }
  
  return;
}

/*
 * TestSkipBlockComment() - Tests whether block comment could be skipped
 */
void TestSkipBlockComment() {
  char data[] = "/******************************\n * This is a comment block \n * \n **//****//*      ****";
  SourceFile sf{data, sizeof(data) - 1};
  
  printf("Test string: %s\n", data);

  try {
    sf.SkipBlockComment();
    sf.SkipBlockComment();
    sf.SkipBlockComment();
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}

/*
 * TestClipIdentifier() - Tests whether identifier could be clipped correctly
 */
void TestClipIdentifier() {
  char data[] = "asdfgh _12345 aa_123 wangziqi2013 _____cxxx11______ sdsdsdsdasd ass";
  SourceFile sf{data, sizeof(data) - 1};

  printf("Test string: %s\n", data);
  try {
    for(int i = 0;i < 7;i++) {
      sf.SkipSpace();
      std::string *s = sf.ClipIdentifier();
      
      printf("Ident = %s\n", s->c_str());
    }
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}

/*
 * TestClipOperator() - Tests ClipOperator function
 */
void TestClipOperator() {
  char data[] = "()<<=>>=[]&&&++++---&^===";
  SourceFile sf{data, sizeof(data) - 1};
  
  printf("Test string: %s\n", data);
  try {
    while(sf.IsEof() == false) {
      TokenType t = sf.ClipOperator();

      printf("TokenType = %d\n", static_cast<int>(t));
    }
  } catch(const std::string &reason) {
    // There will be an error at the last position, on char 0x00
    std::cout << reason << std::endl;

    return;
  }
}

/*
 * TestGetNextToken() - Tests overall functionality
 */
void TestGetNextToken() {
  char data[] = "int main() { printf(\"Hello, world\n\"); return 0; }";
  
  // Must -1 here to cut the trailing 0x00
  SourceFile sf{data, sizeof(data) - 1};
  
  std::cout << "========== TestGetNextToken ==========" << std::endl;
  
  try {
    Token *token_p = nullptr;
    while((token_p = sf.GetNextToken()) != nullptr) {
      TokenType type = token_p->GetType();
      
      printf("TokenType = %d; ", (int)token_p->GetType());

      if(type == TokenType::T_INT_CONST) {
        printf("Int const = %lu\n", token_p->GetIntConst());
      } else if(type == TokenType::T_CHAR_CONST) {
        printf("Char const = %c\n", token_p->GetCharConst());
      } else if(type == TokenType::T_STRING_CONST) {
        printf("String const = \"%s\"\n", token_p->GetStringConst()->c_str());
      } else if(type == TokenType::T_IDENT) {
        printf("Identifier = %s\n", token_p->GetIdentifier()->c_str());
      } else {
        printf("Other token type\n");
      }

      delete token_p;
    }
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;
  }
}
