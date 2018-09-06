
#include "context.h"
#include "syntax.h"

using namespace wangziqi2013;
using namespace cfront;

void Context::InitializeBuiltInTypeMap() {
  for(const auto token_type : TokenInfo::builtin_type_set) {
    // Create a Token node wrapped by a SyntaxNode
    // and insert it into the type map for later use
    builtin_type_map[token_type] = SyntaxNode::Get(Token::Get(token_type));
  }
  
  return;
}
