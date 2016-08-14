
#include "test_suite.h"

/*
 * TestLexClipCharLiteral() - Tests whether chat literal of all escaped
 *                            and non-escaped form could be cliped correctly
 */
void TestLexClipCharLiteral() {
  char data1[] = "'d''\\n''1''\\'''\\x13''\\567''\\\\'";
  
  printf("Test string: %s\n", data1);
  
  SourceFile sf{data1, sizeof(data1)};
  
  for(int i = 0;i < 7;i++) {
    try {
      printf("Char Literal = %d\n", (int)sf.ClipCharLiteral());
    } catch(const std::string &reason) {
      std::cout << reason << std::endl;
      
      return;
    }
  }
  
  return;
}

void TestLexClipStringLiteral() {
  char data[] = "\"This is a string \naaa\tbbb\t\x0anew line";
  
  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data)};
  try {
    printf("String Literal = %s\n", sf.ClipStringLiteral().c_str());
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}
