
CXX=g++
OPT_FLAGS=
CXX_FLAGS=-g -std=c++11
OBJ_FILES=./build/main.o ./build/lex.o ./build/test_suite.o ./build/lex_test.o

all: main
	
main: $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o main $(CXX_FLAGS)

./build/main.o: ./test/main.cpp ./test/test_suite.h ./src/lex.h
	$(CXX) ./test/main.cpp -c -o ./build/main.o $(CXX_FLAGS)
    
./build/test_suite.o: ./test/test_suite.cpp ./test/test_suite.h
	$(CXX) ./test/test_suite.cpp -c -o ./build/test_suite.o $(CXX_FLAGS)
    
./build/lex_test.o: ./test/lex_test.cpp ./test/test_suite.h ./src/lex.h ./src/lex.cpp
	$(CXX) ./test/lex_test.cpp -c -o ./build/lex_test.o $(CXX_FLAGS)
    
./build/lex.o: ./src/lex.h ./src/lex.cpp
	$(CXX) ./src/lex.cpp -c -o ./build/lex.o $(CXX_FLAGS)
    
clean:
	rm ./main
	rm ./build/*