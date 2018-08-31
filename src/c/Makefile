
CC=gcc
CFLAGS=-O0 
LDFLAGS=

all: test_parse_exp

test_parse_exp: test_parse_exp.o parse_exp.o
	$(CC) test_parse_exp.o parse_exp.o -o test_parse_exp $(CFLAGS) $(LDFLAGS)

test_parse_exp.o: test_parse_exp.c
	$(CC) test_parse_exp.c -o test_parse_exp.o $(CFLAGS) $(LDFLAGS)

parse_exp.o: parse_exp.c token.h
	$(CC) parse_exp.c -o parse_exp.o $(CFLAGS) $(LDFLAGS)