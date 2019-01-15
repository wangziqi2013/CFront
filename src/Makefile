
CC=gcc
LD=ld
CFLAGS=-O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
PWD=$(CURDIR)
TESTFLAGS=-I$(PWD)
LDFLAGS=

ifeq ($(OPT), 1)
	CFLAGS=-O3
endif

all: ./bin/test_lex ./bin/test_parse ./bin/test_type ./bin/test_x86

test: all
	./bin/test_lex
	./bin/test_parse
	./bin/test_type
	./bin/test_x86

mem-test: all
	valgrind --leak-check=full ./bin/test_lex
	valgrind --leak-check=full ./bin/test_parse
	valgrind --leak-check=full ./bin/test_type

./bin/test_parse: test_parse.o parse_exp.o stack.o token.o error.o ast.o hashtable.o parse_decl.o parse_comp.o parse_stmt.o parse.o
	$(CC) test_parse.o parse_exp.o stack.o token.o error.o ast.o hashtable.o parse_decl.o parse_comp.o parse_stmt.o parse.o -o ./bin/test_parse $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

./bin/test_lex: test_lex.o stack.o token.o error.o ast.o hashtable.o
	$(CC) test_lex.o stack.o token.o error.o ast.o hashtable.o -o ./bin/test_lex $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

./bin/test_type: test_type.o stack.o token.o error.o ast.o hashtable.o type.o eval.o bintree.o list.o str.o
	$(CC) test_type.o stack.o token.o error.o ast.o hashtable.o type.o eval.o bintree.o list.o str.o -o ./bin/test_type $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

./bin/test_x86: test_x86.o x86.o
	$(CC) test_x86.o x86.o -o ./bin/test_x86 $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

test_lex.o: ./test/test_lex.c token.h
	$(CC) ./test/test_lex.c -c -o $(PWD)/test_lex.o $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

test_parse.o: ./test/test_parse.c token.h
	$(CC) ./test/test_parse.c -c -o $(PWD)/test_parse.o $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

test_type.o: ./test/test_type.c token.h
	$(CC) ./test/test_type.c -c -o $(PWD)/test_type.o $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

test_x86.o: ./test/test_x86.c ./x86/x86.h
	$(CC) ./test/test_x86.c -c -o $(PWD)/test_x86.o $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

parse_exp.o: parse_exp.c parse_exp.h stack.h token.h ast.h hashtable.h error.h parse_decl.h error.h token.h
	$(CC) parse_exp.c -c -o parse_exp.o $(CFLAGS) $(LDFLAGS)

parse_decl.o: parse_decl.c parse_decl.h hashtable.h parse_exp.h error.h token.h
	$(CC) parse_decl.c -c -o parse_decl.o $(CFLAGS) $(LDFLAGS)

parse_comp.o: parse_comp.c parse_comp.h parse_exp.h parse_decl.h token.h
	$(CC) parse_comp.c -c -o parse_comp.o $(CFLAGS) $(LDFLAGS)

parse_stmt.o: parse_stmt.c parse_stmt.h parse_exp.h parse_decl.h parse_comp.h token.h
	$(CC) parse_stmt.c -c -o parse_stmt.o $(CFLAGS) $(LDFLAGS)

parse.o: parse.c parse.h parse_exp.h parse_decl.h parse_comp.h parse_stmt.h
	$(CC) parse.c -c -o parse.o $(CFLAGS) $(LDFLAGS)

stack.o: stack.c stack.h
	$(CC) stack.c -c -o stack.o $(CFLAGS) $(LDFLAGS)

token.o: token.c token.h error.h hashtable.h stack.h
	$(CC) token.c -c -o token.o $(CFLAGS) $(LDFLAGS)

error.o: error.c error.h
	$(CC) error.c -c -o error.o $(CFLAGS) $(LDFLAGS)

ast.o: ast.c ast.h token.h
	$(CC) ast.c -c -o ast.o $(CFLAGS) $(LDFLAGS)

hashtable.o: hashtable.c hashtable.h
	$(CC) hashtable.c -c -o hashtable.o $(CFLAGS) $(LDFLAGS)

bintree.o: bintree.c bintree.h
	$(CC) bintree.c -c -o bintree.o $(CFLAGS) $(LDFLAGS)

str.o: str.c str.h
	$(CC) str.c -c -o str.o $(CFLAGS) $(LDFLAGS)

list.o: list.c list.h
	$(CC) list.c -c -o list.o $(CFLAGS) $(LDFLAGS)

type.o: type.c type.h token.h ast.h hashtable.h stack.h error.h bintree.h list.h eval.h
	$(CC) type.c -c -o type.o $(CFLAGS) $(LDFLAGS)

eval.o: eval.c eval.h token.h type.h
	$(CC) eval.c -c -o eval.o $(CFLAGS) $(LDFLAGS)

x86.o: ./x86/x86.h ./x86/x86.c
	$(CC) x86.c -c -o x86.o $(CFLAGS) $(LDFLAGS)

parse_exp.h: stack.h token.h ast.h hashtable.h

#queue.o: queue.c queue.h
#	$(CC) queue.c -c -o queue.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o
	rm -r ./bin/*
