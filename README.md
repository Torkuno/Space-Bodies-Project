run gtest:

g++ main.cpp -c

g++ main_test.cpp -c

g++ main.o main_test.o -lgtest -lgtest_main -lpthread -o test_executable

./test_executable
