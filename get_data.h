#ifndef GET_DATA_H
#define GET_DATA_H

#include "platform_config.h"
#include <string>

// Function declarations
void loadEnvFile(const std::string& filename);
bool load_from_file(nlohmann::json& jsonData, const std::string& filename);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);
void ask_for_dates(std::string& startDate, std::string& endDate);
void output_neo_data(const nlohmann::json& neo);
void process_neo_data(const nlohmann::json& jsonData);
std::string fetch_neo_data(const std::string& startDate, const std::string& endDate, const std::string& apiKey);

#endif // GET_DATA_H