
CC=gcc
LD=ld
CFLAGS=-O0 
LDFLAGS=

all: ./bin/test_parse_exp

./bin/test_parse_exp: test_parse_exp.o parse_exp.o
	$(CC) test_parse_exp.o parse_exp.o -o ./bin/test_parse_exp $(LDFLAGS)

test_parse_exp.o: test_parse_exp.c
	$(CC) test_parse_exp.c -c -o test_parse_exp.o $(CFLAGS) $(LDFLAGS)

parse_exp.o: parse_exp.c token.h ast.h
	$(CC) parse_exp.c -c -o parse_exp.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o
	rm -r ./bin/*