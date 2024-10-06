# Download libraries

1. Run the following commands in MinGW:

    - cURL:

        `pacman -S mingw-w64-x86_64-curl`

    - nlohmann-json

        `pacman -S mingw-w64-x86_64-nlohmann-json`

2. go to c++ extension

3. go to settings of extension

4. press `extension settings`

5. in the search bar add a space and write ` path`

6. scroll down to `include path`

7. press `add item`

8. add `C:\msys64\mingw64\include`

9. now you can compile the code (it will use `launch.json`, `settings.json` adn `tasks.json` inside `.vscode` folder) and run it.



---


# Run gtest:

```
g++ main.cpp -c

g++ main_test.cpp -c

g++ main.o main_test.o -lgtest -lgtest_main -lpthread -o test_executable


./test_executable
```



# Run main.cpp

g++ main.cpp src/get_data.cpp src/classes.cpp -o main.exe -lcurl
