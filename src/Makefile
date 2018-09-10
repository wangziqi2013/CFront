
CC=gcc
LD=ld
CFLAGS=-O0 -g -Wall -Wextra -Werror
LDFLAGS=

ifeq ($(OPT), 1)
	CFLAGS=-O3
endif

all: ./bin/test_parse_exp

test: all
	./bin/test_parse_exp

mem-test: all
	valgrind --leak-check=full ./bin/test_parse_exp

./bin/test_parse_exp: test_parse_exp.o parse_exp.o stack.o token.o error.o ast.o hashtable.o parse_decl.o parse_comp.o parse_stmt.o
	$(CC) test_parse_exp.o parse_exp.o stack.o token.o error.o ast.o hashtable.o parse_decl.o parse_comp.o parse_stmt.o -o ./bin/test_parse_exp $(CFLAGS) $(LDFLAGS)

test_parse_exp.o: test_parse_exp.c
	$(CC) test_parse_exp.c -c -o test_parse_exp.o $(CFLAGS) $(LDFLAGS)

parse_exp.o: parse_exp.c parse_exp.h stack.h token.h ast.h hashtable.h error.h parse_decl.h error.h
	$(CC) parse_exp.c -c -o parse_exp.o $(CFLAGS) $(LDFLAGS)

parse_decl.o: parse_decl.c parse_decl.h hashtable.h parse_exp.h error.h
	$(CC) parse_decl.c -c -o parse_decl.o $(CFLAGS) $(LDFLAGS)

parse_comp.o: parse_comp.c parse_comp.h parse_exp.h parse_decl.h
	$(CC) parse_comp.c -c -o parse_comp.o $(CFLAGS) $(LDFLAGS)

parse_stmt.o: parse_stmt.c parse_stmt.h parse_exp.h parse_decl.h parse_comp.h
	$(CC) parse_stmt.c -c -o parse_stmt.o $(CFLAGS) $(LDFLAGS)

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