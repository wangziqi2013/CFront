
CC=gcc
LD=ld
CFLAGS=-O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
PWD=$(CURDIR)
TESTFLAGS=-I$(PWD)
LDFLAGS=
BIN=./bin

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.d)

TEST_SRCS=$(wildcard ./tests/*.c)
TEST_OBJS=$(patsubst ./tests/%.c,$(BIN)/%,$(TEST_SRCS))

ifeq ($(OPT), 1)
	CFLAGS=-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
endif

.phony: all tests line-count mem-test clean

all: tests

tests: $(TEST_OBJS)

%.o: %.c
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

./bin/%: ./tests/%.c $(OBJS)
	$(CC) $< $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

-include $(DEPS)

line-count:
	cloc --exclude-lang=Python ./

clean:
	rm -f *.o
	rm -rf $(BIN)/*
