
CXX=g++
OPT_FLAGS=
CXX_FLAGS=-g -std=c++11
OBJ_FILES=./build/main.o ./build/lex.o ./build/test_suite.o ./build/lex_test.o ./build/token.o ./build/syntax.o ./build/syntax_test.o
SRC=$(wildcard ./src/*.cpp) $(wildcard ./src/*.h) $(wildcard ./test/*.cpp)

all: main
	
main: $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o main $(CXX_FLAGS)

./build/main.o: ./test/main.cpp ./test/test_suite.h $(SRC)
	$(CXX) ./test/main.cpp -c -o ./build/main.o $(CXX_FLAGS)
    
./build/test_suite.o: ./test/test_suite.cpp ./test/test_suite.h
	$(CXX) ./test/test_suite.cpp -c -o ./build/test_suite.o $(CXX_FLAGS)
    
./build/lex_test.o: ./test/lex_test.cpp ./test/test_suite.h $(SRC)
	$(CXX) ./test/lex_test.cpp -c -o ./build/lex_test.o $(CXX_FLAGS)
    
./build/lex.o: $(SRC)
	$(CXX) ./src/lex.cpp -c -o ./build/lex.o $(CXX_FLAGS)

./build/token.o: $(SRC)
	$(CXX) ./src/token.cpp -c -o ./build/token.o $(CXX_FLAGS)

./build/syntax.o: $(SRC)
	$(CXX) ./src/syntax.cpp -c -o ./build/syntax.o $(CXX_FLAGS)

./build/syntax_test.o: $(SRC)
	$(CXX) ./test/syntax_test.cpp -c -o ./build/syntax_test.o $(CXX_FLAGS)
    
clean:
	rm -f ./main
	rm -f ./build/*
