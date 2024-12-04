cmake_minimum_required(VERSION 3.10)

# Set the project name
project(NEOAnalyzer)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
file(GLOB SOURCES "src/*.cpp" "src/*.h")
add_executable(NEOAnalyzer main.cpp ${SOURCES})

# Set the prefix path for nlohmann_json conditionally
if(APPLE)
    set(CMAKE_PREFIX_PATH "/usr/local/opt/nlohmann-json")
endif()

# Find and include the nlohmann_json library
find_package(nlohmann_json REQUIRED)
target_link_libraries(NEOAnalyzer PRIVATE nlohmann_json::nlohmann_json)

find_package(CURL REQUIRED)
target_link_libraries(NEOAnalyzer PRIVATE CURL::libcurl)

# Find and include the SFML library
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(NEOAnalyzer PRIVATE sfml-graphics sfml-window sfml-system)

# Include directories
target_include_directories(NEOAnalyzer PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Copy .env file to the build directory
configure_file(${CMAKE_SOURCE_DIR}/.env ${CMAKE_BINARY_DIR}/.env COPYONLY)
