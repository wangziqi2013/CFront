
#include "test_suite.h"

/*
 * TestParseExpression1() - Tests whether the basic stuff is working or not
 */
void TestParseExpression1() {
  printf("========== TestParseExpression1 ==========\n");
  
  char data[] = "(123456 + *a) * &*++*b++ += cde++++";

  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  Context context{};
  ExpressionParser sa{&sf, &context};
  
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
  Context context{};
  ExpressionParser sa{&sf, &context};

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
  Context context{};
  ExpressionParser sa{&sf, &context};

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
  Context context{};
  ExpressionParser sa{&sf, &context};

  try {
    SyntaxNode *node_p = sa.ParseExpression();

    node_p->TraversePrint();
  } catch(const std::string &reason) {
    std::cout << reason << std::endl;

    return;
  }
}

/*
 * TestParseType1() - Testing parsing signed and unsigned integral types
 */
void TestParseType1() {
  printf("========== TestParseType1 ==========\n");

  char data[] = "unsigned long     signed int    int  "
                "   unsigned char char signed short ";

  printf("Test string: %s\n", data);

  SourceFile sf{data, sizeof(data) - 1};
  Context context{};
  TypeParser ta{&sf, &context};

  try {
    SyntaxNode *node_p = nullptr;
    
    node_p = ta.TryParseBaseType();
    assert(node_p->GetType() == TokenType::T_ULONG);

    node_p = ta.TryParseBaseType();
    assert(node_p->GetType() == TokenType::T_INT);
    
    node_p = ta.TryParseBaseType();
    assert(node_p->GetType() == TokenType::T_INT);
    
    node_p = ta.TryParseBaseType();
    assert(node_p->GetType() == TokenType::T_UCHAR);
    
    node_p = ta.TryParseBaseType();
    assert(node_p->GetType() == TokenType::T_CHAR);
    
    node_p = ta.TryParseBaseType();
    assert(node_p->GetType() == TokenType::T_SHORT);
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
  
  TestParseType1();
  
  return 0;
}
