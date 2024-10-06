#include <iostream>
#include <string>
#include "get_data.h"

using namespace std;

int main() {
    loadEnvFile(".env");
    string startDate, endDate;  // Declare variables for date input
    cout << "\nWelcome to the NEO Analyzer!" << endl;

    // Ask user for start and end date
    ask_for_dates(startDate, endDate);

    const char* apiKeyEnv = std::getenv("API_KEY");  // Get API key from environment variable
    string apiKey = apiKeyEnv ? apiKeyEnv : "";      // If environment variable is missing, use empty string

    if (apiKey.empty()) {
        cerr << "API key is missing. Please set the API_KEY environment variable." << endl;
        return 1;  // Exit program if API key is not set
    }

    // Fetch NEO data using the provided dates and API key
    string neo_data = fetch_neo_data(startDate, endDate, apiKey);

    if (neo_data.empty()) {
        cerr << "Failed to fetch data from NASA API. Loading data from file..." << endl;

        nlohmann::json jsonData;  // JSON object to store loaded data
        if (!load_from_file(jsonData, "data.json")) {
            cerr << "Failed to load data from file." << endl;
            return 1;  // Exit if both API and file loading fail
        }

        process_neo_data(jsonData);  // Process data from file
    } else {
        try {
            nlohmann::json jsonData = nlohmann::json::parse(neo_data);  // Parse the JSON response
            process_neo_data(jsonData);  // Process the data fetched from API
        } catch (const exception& e) {
            cerr << "Error parsing data: " << e.what() << endl;
            return 1;
        }
    }

    return 0;
}
