#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

// Conditional compilation for different platforms
// Include the JSON library
#if defined(_WIN32) || defined(_WIN64)
    #include <nlohmann/json.hpp>
    #define OS_NAME "Windows"
#elif defined(__APPLE__) || defined(__MACH__)
    #include <nlohmann/json.hpp>
    #define OS_NAME "macOS"
#elif defined(__linux__)
    #include <nlohmann/json.hpp>
    #define OS_NAME "Linux"
#else
    #define OS_NAME "Unknown platform"
#endif

using namespace nlohmann;  // JSON namespace used globally

#endif // PLATFORM_CONFIG_H
