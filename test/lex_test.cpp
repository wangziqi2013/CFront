
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
  
  SourceFile sf{data1, sizeof(data1)};
  
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

  SourceFile sf{data, sizeof(data)};
  try {
    auto ret = sf.ClipStringLiteral();
    auto ret2 = sf.ClipStringLiteral();
    printf("String Literal = %s\n", ret.c_str());
    printf("String Literal = %s\n", ret2.c_str());
    
    assert((AppendDoubleQuote(ret) +
            AppendDoubleQuote(ret2)) == std::string{data});
    
    
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
  
  SourceFile sf{data, sizeof(data)};
  
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
