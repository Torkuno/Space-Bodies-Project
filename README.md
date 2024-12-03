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


1. Open **VSCode** and go to **Settings** (Ctrl + ,).
2. Search for `include path`.
3. Scroll down to the **Include Path** section and press `Add Item`.
4. Add the following address:

  _Windows_
   ```
   C:\msys64\mingw64\include
   ```
_MacOS_
   ```
   /opt/homebrew/include
   ```
5. Now, you can compile the code using the `.vscode` configuration (like `launch.json`, `settings.json`, `tasks.json`).


8. add `C:\msys64\mingw64\include`

9. now you can compile the code (it will use `launch.json`, `settings.json` adn `tasks.json` inside `.vscode` folder) and run it.

---

To run the near-Earth object analyzer, you have two options:

- You can build and run the app directly from VSCode.

- You can build and run the app from the command line.

Use the following commands if you decide to build the app from the command line:

_Windows_
```bash
g++ -fdiagnostics-color=always -g main.cpp src/get_data.cpp -o main.exe -lcurl -lsfml-graphics -lsfml-window -lsfml-system
```
```
main.exe
```

_MacOS_
```bash
g++ -std=c++11 main.cpp src/get_data.cpp -o app -I/opt/homebrew/opt/sfml/include -L/opt/homebrew/opt/sfml/lib -lsfml-graphics -lsfml-window -lsfml-system -lcurl
```
```
./app
```

# Run gtest:

```
g++ main.cpp -c

g++ main_test.cpp -c

g++ main.o main_test.o -lgtest -lgtest_main -lpthread -o test_executable


./test_executable
```



# Run main.cpp

g++ main.cpp src/get_data.cpp -o main.exe -lcurl

g++ -std=c++17 -fdiagnostics-color=always -g main.cpp src/get_data.cpp -o main -lcurl