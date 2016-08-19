
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
  
  ///////////////////////////////////////////////////////////////////
  // Test parse expression
  ///////////////////////////////////////////////////////////////////
  TestParseExpression1();
  TestParseExpression2();
  
  return 0;
}
