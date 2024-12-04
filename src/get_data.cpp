#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <cstdlib>
#include "get_data.h"
#include <exception>

using namespace std;

// Custom Exceptions for file not found and API request errors
class FileNotFoundException : public std::exception {
private:
    std::string message_;
public:
    FileNotFoundException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override {
        return message_.c_str();
    }
};

class ApiRequestException : public std::exception {
private:
    std::string message_;
public:
    ApiRequestException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override {
        return message_.c_str();
    }
};

// Function to load environment variables from a file
void loadEnvFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw FileNotFoundException("Could not open .env file: " + filename);
    }

    string line;
    while (getline(file, line)) {
        // Split line into key=value
        auto delimiterPos = line.find('=');
        if (delimiterPos == string::npos) continue; // Skip invalid lines
        string key = line.substr(0, delimiterPos);
        string value = line.substr(delimiterPos + 1);

#ifdef _WIN32
        _putenv_s(key.c_str(), value.c_str());  // For Windows
#else
        setenv(key.c_str(), value.c_str(), 1);  // For Linux/macOS
#endif
    }
    file.close();
}

// Callback function to capture the response data from cURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s) {
    size_t totalSize = size * nmemb;
    s->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to fetch NEO data from NASA API
string fetch_neo_data(const string& date, const string& apiKey) {
    CURL* curl;
    CURLcode res;
    string neo_data;

    try {
        curl = curl_easy_init();
        if (curl) {
            string baseUrl = "https://api.nasa.gov/neo/rest/v1/feed";
            string url = baseUrl + "?start_date=" + date + "&end_date=" + date + "&api_key=" + apiKey;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &neo_data);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                throw ApiRequestException("cURL error: " + string(curl_easy_strerror(res)));
            }

            curl_easy_cleanup(curl);
        } else {
            throw ApiRequestException("Failed to initialize cURL");
        }

    } catch (const ApiRequestException& e) {
        // Ensure we clean up resources properly
        if (curl) {
            curl_easy_cleanup(curl);
        }
        // Rethrow the caught exception to the higher level
        throw;
    }

    return neo_data;
}


// Function to load data from a local JSON file
bool load_from_file(json& jsonData, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw FileNotFoundException("Could not open file: " + filename);
    }

    try {
        file >> jsonData;
    } catch (const exception& e) {
        file.close();
        throw;  // Rethrow to allow higher-level handling
    }

    file.close();
    return true;
}

// Function to output NEO data (for debugging purposes)
void output_neo_data(const json& neo) {
    try {
        // Extract basic information
        string id = neo["id"];
        string name = neo["name"];
        string nasa_jpl_url = neo["nasa_jpl_url"];
        double absolute_magnitude = neo["absolute_magnitude_h"];

        cout << "NEO ID: " << id << endl;
        cout << "Name: " << name << endl;
        cout << "NASA JPL URL: " << nasa_jpl_url << endl;
        cout << "Absolute Magnitude (H): " << absolute_magnitude << endl;

        // Extract diameter information
        auto diameter = neo["estimated_diameter"];
        cout << "\nEstimated Diameter:" << endl;
        cout << "Kilometers: " << diameter["kilometers"]["estimated_diameter_min"] << " - "
             << diameter["kilometers"]["estimated_diameter_max"] << " km" << endl;
        cout << "Meters: " << diameter["meters"]["estimated_diameter_min"] << " - "
             << diameter["meters"]["estimated_diameter_max"] << " m" << endl;

        // Check if it's potentially hazardous
        bool is_potentially_hazardous = neo["is_potentially_hazardous_asteroid"];
        cout << "\nIs Potentially Hazardous: " << (is_potentially_hazardous ? "Yes" : "No") << endl;

        // Extract close approach data
        auto close_approach = neo["close_approach_data"][0];  // Get the first close approach data
        cout << "\nClose Approach Data:" << endl;
        cout << "Close Approach Date: " << close_approach["close_approach_date"] << endl;
        cout << "Full Close Approach Date: " << close_approach["close_approach_date_full"] << endl;
        cout << "Relative Velocity (km/s): " << close_approach["relative_velocity"]["kilometers_per_second"] << " km/s" << endl;
        cout << "Relative Velocity (km/h): " << close_approach["relative_velocity"]["kilometers_per_hour"] << " km/h" << endl;
        cout << "Miss Distance (Astronomical): " << close_approach["miss_distance"]["astronomical"] << " au" << endl;
        cout << "Miss Distance (Lunar): " << close_approach["miss_distance"]["lunar"] << " lunar distances" << endl;
        cout << "Miss Distance (Kilometers): " << close_approach["miss_distance"]["kilometers"] << " km" << endl;
        cout << "Miss Distance (Miles): " << close_approach["miss_distance"]["miles"] << " miles" << endl;
        cout << "Orbiting Body: " << close_approach["orbiting_body"] << endl;

        // Check if it's a Sentry object
        bool is_sentry_object = neo["is_sentry_object"];
        cout << "\nIs Sentry Object: " << (is_sentry_object ? "Yes" : "No") << endl;

    } catch (const exception&) {
        throw;  // Rethrow exception for higher-level handling
    }
}

// Function to validate menu choice (3 tries allowed)
int validateMenuChoice(int min, int max) {
    int choice;
    for (int attempts = 1; attempts <= 3; ++attempts) {
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cerr << "Invalid input. Please enter an integer value. (" << attempts << "/3 tries)" << endl;
        } else {
            if (choice >= min && choice <= max) {
                return choice; // Valid input
            } else {
                cerr << "Input out of range. Please enter a number between " << min << " and " << max << ". (" << attempts << "/3 tries)" << endl;
            }
        }
        if (attempts == 3) {
            throw runtime_error("Exceeded the maximum number of attempts for menu input.");
        }
    }
    return min; // Default return to prevent compilation issues (logic will always return before this line)
}

// Function to process NEO data from NASA API or local file
json process_neo_data(const json& jsonData, const string& selectedDate) {
    auto& neo_objects = jsonData["near_earth_objects"];
    if (neo_objects.find(selectedDate) == neo_objects.end()) {
        cout << "No NEO data found for the selected date: " << selectedDate << endl;
        return {};  // Instead of throwing, return an empty JSON object to handle retries.
    }

    auto& neos = neo_objects[selectedDate];
    if (neos.empty()) {
        cout << "No NEOs found for the selected date: " << selectedDate << endl;
        return {};  // Instead of throwing, return an empty JSON object to handle retries.
    }

    cout << "\nThere are " << neos.size() << " NEOs for the date " << selectedDate << ".\n";
    for (size_t i = 0; i < neos.size(); i++) {
        cout << i + 1 << ". " << neos[i]["name"] << endl;
    }

    cout << "\nSelect a NEO by number: ";
    return neos[validateMenuChoice(1, neos.size()) - 1]; // Use validateMenuChoice for input validation.
}