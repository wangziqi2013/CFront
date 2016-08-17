
#include "test_suite.h"
#include "../src/lex.h"

int main() {
  TestLexClipCharLiteral();
  TestLexClipStringLiteral();
  TestLexClipIntegerLiteral();
  TestSkipBlockComment();
  TestClipIdentifier();
  TestClipOperator();
  
  TestGetNextToken();
  
  return 0;
}
