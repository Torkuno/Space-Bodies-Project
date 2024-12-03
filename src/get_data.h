#ifndef GET_DATA_H
#define GET_DATA_H

#include "platform_config.h" // Assuming platform-specific configurations
#include <string>

// Function declarations

// Loads environment variables from a file
void loadEnvFile(const std::string& filename);

// Loads NEO data from a file into a json object
bool load_from_file(nlohmann::json& jsonData, const std::string& filename);

// Callback for writing API response data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);

// Outputs the Near-Earth Object data to the console
void output_neo_data(const nlohmann::json& neo);

// Processes the NEO data for a specific date
nlohmann::json process_neo_data(const nlohmann::json& jsonData, const std::string& selectedDate);

// Fetches NEO data from NASA's API for a specific date
std::string fetch_neo_data(const std::string& date, const std::string& apiKey);

#endif // GET_DATA_H