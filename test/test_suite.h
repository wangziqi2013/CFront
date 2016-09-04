
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include <string>
#include <vector>
#include <iostream>

// It is safe to put it here since we only use the header file for testing
using namespace wangziqi2013;
using namespace cfront;

#include "../src/lex.h"
#include "../src/token.h"
#include "../src/syntax.h"

std::string AppendDoubleQuote(std::string s);

void TestLexClipCharLiteral();
void TestLexClipStringLiteral();
void TestLexClipIntegerLiteral();
void TestSkipBlockComment();
void TestClipIdentifier();
void TestClipOperator();
void TestGetNextToken();

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void TestParseExpression1();
