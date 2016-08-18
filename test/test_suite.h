
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include <string>
#include <vector>
#include <iostream>

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
