
CC=gcc
LD=ld
CFLAGS=-O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
PWD=$(CURDIR)
TESTFLAGS=-I$(PWD)
LDFLAGS=
BIN=./bin/

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.d)

%.o: %.c
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

-include $(DEPS)

ifeq ($(OPT), 1)
	CFLAGS=-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
endif

.phony: all test line-count mem-test clean

all: $(BIN)/test_lex $(BIN)/test_parse $(BIN)/test_type $(BIN)/test_eval $(BIN)/test_cgen

line-count:
	cloc --exclude-lang=Python ./

test: all
	$(BIN)/test_lex
	$(BIN)/test_parse
	$(BIN)/test_type
	$(BIN)/test_eval
	$(BIN)/test_cgen

mem-test: all
	valgrind --leak-check=full $(BIN)/test_lex
	valgrind --leak-check=full $(BIN)/test_parse
	valgrind --leak-check=full $(BIN)/test_type
	valgrind --leak-check=full $(BIN)/test_eval
	valgrind --leak-check=full $(BIN)/test_cgen

$(BIN)/test_parse: ./tests/test_parse.c $(OBJS)
	$(CC) ./tests/test_parse.c $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

$(BIN)/test_lex: ./tests/test_lex.c $(OBJS)
	$(CC) ./tests/test_lex.c $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

$(BIN)/test_type: ./tests/test_type.c $(OBJS)
	$(CC) ./tests/test_type.c $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)
	
$(BIN)/test_eval: ./tests/test_eval.c $(OBJS)
	$(CC) ./tests/test_eval.c $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

$(BIN)/test_cgen: ./tests/test_cgen.c $(OBJS)
	$(CC) ./tests/test_cgen.c $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

clean:
	rm -f *.o
	rm -rf $(BIN)/*
