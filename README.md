# CFront  
The goal of this project is to build a C compiler from the scratch without using any third-party code except standard C library.   
  
# Directory Structure  
[./src](https://github.com/wangziqi2013/CFront/tree/master/src) - Main source directory
   
[./src/test](https://github.com/wangziqi2013/CFront/tree/master/src/test) - Unit tests and functional tests
  
[./src/old](https://github.com/wangziqi2013/CFront/tree/master/src/old) - Deprecated code. Only for demonstration purposes.
 
[./src/python](https://github.com/wangziqi2013/CFront/tree/master/src/python) - A LL(1)/LR(1)/LALR(1) compiler generator implemented in Python

# Source File Description

## Main Files

./src/token.c: Implements lexical analysis and the token stream interface

./src/parse_exp.c: Implements parsing interface and expression parsing. The entire parser is based on expression parsing, which uses a hand-coded shift-reduce parser with operator precedence.

./src/parse_decl.c: Implements declaration parsing. It uses expression parsing to build declaration tree (in C language, declaration has exactly the same format as an expression).

./src/parse_comp.c: Implements composite type declaration parsing, including struct, union and enum.

./src/parse_stmt.c: Implements statement parsing.

./src/parse.c: Implements top-level (global declaration, definition and function definition) parsing.

./src/type.c: Implements the type system.

./src/eval.c: Implements compile-time evaluation support, including constant evaluation, atoi, string to binary, etc.

./src/cgen.c: Implements top-level code generation.
 
## Data Structure Files  
 
./src/ast.c: Implements abstract syntax tree. We use left-child right-sibling organization for trees.

./src/str.c: Implements vector and string.

./src/hashtable.c: Implements hash table. We use hash table as symbol tables for scopes.

./src/bintree.c: Implements a simple binary search tree. We use binary search trees as indices for composite types.

./src/list.c: Implements singly linked list.

./src/stack.c: Implements a stack. We use stack to maintain scopes and to perform shift-reduce parsing.
 
# Compile and Test
To compile, enter ./src directory, and type `make all` or just `make`. This will build object files for each source file, and link them with the tests.

To test, directly run binary under ./bin directory. Test source files are independent from each other (i.e. there is no mutral dependency), and should be rather straightforward to understand.

# Contribution
I only contribute to this project in my part-time. If you are interested in becoming a contributor feel free to drop me a message on Github.
