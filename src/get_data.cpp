#include "get_data.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <cstdlib>

using namespace std;

// Function to load environment variables from a file
void loadEnvFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open .env file" << endl;
        return;
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

    curl = curl_easy_init();
    if (curl) {
        string baseUrl = "https://api.nasa.gov/neo/rest/v1/feed";
        string url = baseUrl + "?start_date=" + date + "&end_date=" + date + "&api_key=" + apiKey;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &neo_data);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "cURL request failed: " << curl_easy_strerror(res) << endl;
            neo_data = "";  // Return empty string if the request fails
        }

        curl_easy_cleanup(curl);
    }
    return neo_data;
}

// Function to load data from a local JSON file
bool load_from_file(json& jsonData, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open the file!" << endl;
        return false;
    }
    file >> jsonData;
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

    } catch (const exception& e) {
        cerr << "Error processing NEO data: " << e.what() << endl;
    }
}

// Function to process NEO data from NASA API or local file
json process_neo_data(const json& jsonData, const string& selectedDate) {
    try {
        auto& neo_objects = jsonData["near_earth_objects"];
        if (neo_objects.contains(selectedDate)) {
            auto neos = neo_objects[selectedDate];
            cout << "\nThere are " << neos.size() << " NEOs for the date " << selectedDate << ".\n";

            for (size_t i = 0; i < neos.size(); i++) {
                cout << i + 1 << ". " << neos[i]["name"] << endl;
            }

            int neo_choice;
            cout << "\nSelect a NEO by number: ";
            cin >> neo_choice;

            if (neo_choice > 0 && neo_choice <= neos.size()) {
                return neos[neo_choice - 1]; // Return the selected NEO JSON object
            } else {
                cout << "Invalid choice, please select a valid NEO number." << endl;
            }
        } else {
            cout << "No NEO data available for the selected date." << endl;
            return {}; // Return an empty JSON object
        }
    } catch (const exception& e) {
        cerr << "Error processing data: " << e.what() << endl;
        return {}; // Return an empty JSON object
    }
    // Default return in case none of the above conditions are met
    return {};
}
