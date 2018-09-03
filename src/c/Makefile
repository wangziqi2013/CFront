
CC=gcc
LD=ld
CFLAGS=-O0 -g
LDFLAGS=

all: ./bin/test_parse_exp

./bin/test_parse_exp: test_parse_exp.o parse_exp.o stack.o token.o error.o ast.o hashtable.o
	$(CC) test_parse_exp.o parse_exp.o stack.o token.o error.o ast.o hashtable.o -o ./bin/test_parse_exp $(CFLAGS) $(LDFLAGS)

test_parse_exp.o: test_parse_exp.c
	$(CC) test_parse_exp.c -c -o test_parse_exp.o $(CFLAGS) $(LDFLAGS)

parse_exp.o: parse_exp.c token.h ast.h stack.h parse_exp.h
	$(CC) parse_exp.c -c -o parse_exp.o $(CFLAGS) $(LDFLAGS)

stack.o: stack.c stack.h
	$(CC) stack.c -c -o stack.o $(CFLAGS) $(LDFLAGS)

token.o: token.c token.h
	$(CC) token.c -c -o token.o $(CFLAGS) $(LDFLAGS)

error.o: error.c error.h
	$(CC) error.c -c -o error.o $(CFLAGS) $(LDFLAGS)

ast.o: ast.c ast.h
	$(CC) ast.c -c -o ast.o $(CFLAGS) $(LDFLAGS)

hashtable.o: hashtable.c hashtable.h
	$(CC) hashtable.c -c -o hashtable.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o
	rm -r ./bin/*