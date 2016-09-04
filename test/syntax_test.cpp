
#include "test_suite.h"

/*
 * TestParseExpression1() - Tests whether the basic stuff is working or not
 */
void TestParseExpression1() {
  printf("========== TestParseExpression1 ==========\n");
  
  char data[] = "(123456 + *a) * &*++*b++ += cde++++";

  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  ExpressionParser sa{&sf};
  
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
  printf("========== TestParseExpression2 ==========\n");

  char data[] = "123 + 456--++ - 789 * 100";

  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  ExpressionParser sa{&sf};

  try {
    SyntaxNode *node_p = sa.ParseExpression();

    node_p->TraversePrint();
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}

/*
 * TestParseExpression3() - Tests array sub
 */
void TestParseExpression3() {
  printf("========== TestParseExpression3 ==========\n");

  char data[] = "*(((c)) = (a))++[b]-- + d[123]";

  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  ExpressionParser sa{&sf};

  try {
    SyntaxNode *node_p = sa.ParseExpression();

    node_p->TraversePrint();
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}

/*
 * TestParseExpression4() - Tests function call
 */
void TestParseExpression4() {
  printf("========== TestParseExpression4 ==========\n");

  char data[] = "(*func(888, 666, (wzq())))--";

  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  ExpressionParser sa{&sf};

  try {
    SyntaxNode *node_p = sa.ParseExpression();

    node_p->TraversePrint();
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}

int main() {
  TestParseExpression1();
  TestParseExpression2();
  TestParseExpression3();
  TestParseExpression4();
  
  return 0;
}
