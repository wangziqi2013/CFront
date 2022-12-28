
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

# Build rule for source files under the current directory, one object file per source file
%.o: %.c
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

# Build rule for test source files under ./tests directory, one binary per test source file
./bin/%: ./tests/%.c $(OBJS)
	$(CC) $< $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTFLAGS)

# Include automatically generated dependency files for every source file
-include $(DEPS)

line-count:
	cloc --exclude-lang=Python ./

clean:
	rm -f *.o
	rm -f $(BIN)/*
