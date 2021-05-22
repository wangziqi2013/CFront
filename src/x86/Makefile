
CFLAGS=-O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
LDFLAGS=

all: x86-test

x86-test: x86.o x86-test.c
	gcc x86-test.c -o x86-test $(CFLAGS) $(LDFLAGS)

x86.o: x86.c x86.h
	gcc x86.c -c -o x86.o $(CFLAGS) $(LDFLAGS)
