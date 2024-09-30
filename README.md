# Download libraries

Run the following commands in MinGW

## cURL
`pacman -S mingw-w64-x86_64-curl`

## nlohmann-json
`pacman -S mingw-w64-x86_64-nlohmann-json`

# Run gtest:

g++ main.cpp -c

g++ main_test.cpp -c

g++ main.o main_test.o -lgtest -lgtest_main -lpthread -o test_executable

./test_executable
