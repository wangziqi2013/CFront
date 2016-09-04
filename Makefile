
CXX=g++
OPT_FLAGS=
CXX_FLAGS=-g -std=c++11
OBJ= ./build/lex.o ./build/token.o ./build/syntax.o ./build/context.o ./build/scope.o ./build/test_suite.o
SRC=$(wildcard ./src/*.cpp)
HDR=$(wildcard ./src/*.h)
ALL_FILES=$(SRC) $(HDR) $(wildcard ./test/*.h) $(wildcard ./test/*.cpp)

all:
	@echo "*"
	@echo "* Please run make + test name to build a specific test!"
	@echo "*"
    
./build/lex.o: $(SRC) $(HDR)
	$(CXX) ./src/lex.cpp -c -o ./build/lex.o $(CXX_FLAGS)

./build/token.o: $(SRC) $(HDR)
	$(CXX) ./src/token.cpp -c -o ./build/token.o $(CXX_FLAGS)

./build/syntax.o: $(SRC) $(HDR)
	$(CXX) ./src/syntax.cpp -c -o ./build/syntax.o $(CXX_FLAGS)
    
./build/context.o: $(SRC) $(HDR)
	$(CXX) ./src/context.cpp -c -o ./build/context.o $(CXX_FLAGS)
    
./build/scope.o: $(SRC) $(HDR)
	$(CXX) ./src/scope.cpp -c -o ./build/scope.o $(CXX_FLAGS)

./build/test_suite.o: ./test/test_suite.cpp ./test/test_suite.h
	$(CXX) ./test/test_suite.cpp -c -o ./build/test_suite.o $(CXX_FLAGS)
    
lex_test: $(OBJ) ./test/lex_test.cpp
	$(CXX) ./test/lex_test.cpp -c -o ./build/lex_test.o $(CXX_FLAGS)
	$(CXX) $(OBJ) ./build/lex_test.o -o ./bin/lex_test $(CXX_FLAGS)
    
syntax_test: $(OBJ) ./test/syntax_test.cpp
	$(CXX) ./test/syntax_test.cpp -c -o ./build/syntax_test.o $(CXX_FLAGS)
	$(CXX) $(OBJ) ./build/syntax_test.o -o ./bin/syntax_test $(CXX_FLAGS)
 
clean:
	rm -f ./main
	rm -f ./build/*
