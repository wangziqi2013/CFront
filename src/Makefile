
CC=gcc
LD=ld
CFLAGS=-O0 -g -Wall -Wextra -Werror
LDFLAGS=

ifeq ($(OPT), 1)
	CFLAGS=-O3
endif

all: ./bin/test_parse

test: all
	./bin/test_parse

mem-test: all
	valgrind --leak-check=full ./bin/test_parse

./bin/test_parse: test_parse.o parse_exp.o stack.o token.o error.o ast.o hashtable.o parse_decl.o parse_comp.o parse_stmt.o parse.o
	$(CC) test_parse.o parse_exp.o stack.o token.o error.o ast.o hashtable.o parse_decl.o parse_comp.o parse_stmt.o parse.o -o ./bin/test_parse $(CFLAGS) $(LDFLAGS)

./bin/test_lex: test_lex.o stack.o token.o error.o
	$(CC) test_parse.o stack.o token.o error.o -o ./bin/test_lex $(CFLAGS) $(LDFLAGS)

test_parse.o: ./test/test_parse.c token.h
	$(CC) ./test/test_parse.c -c -o test_parse.o $(CFLAGS) $(LDFLAGS)

test_lex.o: ./test/test_lex.c token.h
	$(CC) ./test/test_lex.c -c -o test_lex.o $(CFLAGS) $(LDFLAGS)

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

ast.o: ast.c ast.h
	$(CC) ast.c -c -o ast.o $(CFLAGS) $(LDFLAGS)

hashtable.o: hashtable.c hashtable.h
	$(CC) hashtable.c -c -o hashtable.o $(CFLAGS) $(LDFLAGS)

parse_exp.h: stack.h token.h ast.h hashtable.h

#queue.o: queue.c queue.h
#	$(CC) queue.c -c -o queue.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o
	rm -r ./bin/*