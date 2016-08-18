
#include "test_suite.h"

void TestParseExpression1() {
  printf("========== TestParseExpression1 ==========\n");
  
  char data[] = "123456 + a * &*++*b++ += cde";

  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  SyntaxAnalyzer sa{&sf};
  
  try {
    SyntaxNode *node_p = sa.ParseExpression();
    
    node_p->TraversePrint();
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}
