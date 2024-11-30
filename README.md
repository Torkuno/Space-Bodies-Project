# **Asteroid Orbit Visualization Project**

This project visualizes the orbit of near-Earth objects (NEOs) around Earth using data fetched from NASA's Near-Earth Object Web Service (NeoWs). It simulates the elliptical orbit of an asteroid and allows you to interact with the orbit using a slider to adjust the time elapsed during the orbit.

## **Project Structure**

- **main.cpp**: Contains the main logic to fetch data, compute asteroid orbits, and render the visualization using SFML (Simple and Fast Multimedia Library).
- **src/get_data.cpp / get_data.h**: Manages fetching NEO data from NASA's API using cURL.
- **json.hpp**: Header-only library to handle JSON data.
- **Earth_Image.jpeg**: Image used as the texture for the Earth in the visualization.
- **.vscode**: Contains configuration files for compiling and running the code within Visual Studio Code.

## **Installation Guide**

### 1. **Install Dependencies**

To successfully compile and run this project, you need to install several dependencies, including `SFML`, `cURL`, and `nlohmann-json`.

- **On MinGW** (Windows):
    - Install cURL:
      ```bash
      pacman -S mingw-w64-x86_64-curl
      ```
    - Install nlohmann-json:
      ```bash
      pacman -S mingw-w64-x86_64-nlohmann-json
      ```

- **On macOS**:
    - Install Homebrew if you haven’t already:
      ```bash
      /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
      ```
    - Install SFML and cURL:
      ```bash
      brew install sfml curl
      ```
    - Install nlohmann-json:
      ```bash
      brew install nlohmann-json
      ```

### 2. **Configure Visual Studio Code**

This project is configured to be compiled and run from VSCode. Follow these steps to set up the environment:

1. Open **VSCode** and go to **Settings** (Ctrl + ,).
2. Search for `include path`.
3. Scroll down to the **Include Path** section and press `Add Item`.
4. Add:
   ```
   C:\msys64\mingw64\include  # For Windows
   /opt/homebrew/include  # For macOS
   ```
5. Now, you can compile the code using the `.vscode` configuration (like `launch.json`, `settings.json`, `tasks.json`).

### 3. Get NASA API Key
To complete this step and be able to use this program visit [NASA API](https://api.nasa.gov/) and get your API key. Once done, create a `.env` file in the root directory and add the following line:
```
API_KEY=your_api_key
```

## **Running the Application**

### 1. **Run the Main Application**

To run the main application that visualizes the asteroid's orbit around Earth:

```bash
g++ -std=c++11 main.cpp src/get_data.cpp -o app -I/opt/homebrew/opt/sfml/include -L/opt/homebrew/opt/sfml/lib -lsfml-graphics -lsfml-window -lsfml-system -lcurl

./app
```

### 2. **Running Tests (Optional)**

If you have unit tests written for the project using Google Test (`gtest`), you can go to googletest brunch

## **Features**

- **Real-time Visualization**: Watch the asteroid orbit around Earth using a dynamic slider to control time elapsed.
- **Data Fetching**: Fetches real-time data of Near-Earth Objects from NASA's NeoWs API.
- **Elliptical Orbit Simulation**: Simulates elliptical orbits around Earth using astrodynamics calculations.

## **How It Works**

1. **Fetch Asteroid Data**: The application fetches a list of NEOs for a selected date from NASA's API.
2. **Select Asteroid**: You can select which asteroid to visualize.
3. **Orbit Calculation**: The asteroid's orbit is calculated based on its close approach distance, velocity, and other parameters.
4. **Visualize Orbit**: The orbit is displayed with Earth at the center, and the user can control the asteroid's position using the slider.

## **Technologies Used**

- **SFML**: For graphics and window handling.
- **cURL**: To fetch data from NASA's API.
- **nlohmann-json**: To parse JSON data from the API.
- **Google Test (Optional)**: For unit testing (if applicable).

## **Contributing**

If you'd like to contribute to this project, feel free to fork the repository, create a feature branch, and submit a pull request.
