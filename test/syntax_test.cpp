
#include "test_suite.h"

/*
 * TestParseExpression1() - Tests whether the basic stuff is working or not
 */
void TestParseExpression1() {
  printf("========== TestParseExpression1 ==========\n");
  
  char data[] = "(123456 + *a) * &*++*b++ += cde++++";

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

/*
 * TestParseExpression2() - Tests associativity
 */
void TestParseExpression2() {
  printf("========== TestParseExpression1 ==========\n");

  char data[] = "123 + 456 - 789";

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
