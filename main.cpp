#include <iostream>
#include <curl/curl.h>
#include <string>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <cstdlib>

// Conditional compilation for different platforms
// Include the JSON library
#if defined(_WIN32) || defined(_WIN64)
    #include <nlohmann/json.hpp>
    #define OS_NAME "Windows"
#elif defined(__APPLE__) || defined(__MACH__)
    #include "json.hpp" // Include for macOS
    #define OS_NAME "macOS"
#elif defined(__linux__)
    #include "json.hpp" // Include for Linux
    #define OS_NAME "Linux"
#else
    #define OS_NAME "Unknown platform"
#endif

using namespace nlohmann;
using namespace std;

void loadEnvFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open .env file" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Split line into key=value
        auto delimiterPos = line.find('=');
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);


#ifdef _WIN32
        _putenv_s(key.c_str(), value.c_str());  // For Windows
#else
        setenv(key.c_str(), value.c_str(), 1);  // For Linux/macOS
#endif
    }
    file.close();
}

class OrbitCalculations {
public:
    // Nested class for perihelion and aphelion calculations
    class PerihelionAphelion {
    public:
        static double calculate_perihelion(double semi_major_axis, double eccentricity);
        static double calculate_aphelion(double semi_major_axis, double eccentricity);
    };

    // Nested class for velocity calculations
    class Velocity {
    public:
        static double calculate_velocity_at_perihelion(double semi_major_axis);
    };
};

class CloseApproachAnalysis {
public:
    static double calculate_minimum_distance_to_earth(double neo_params);
};

class ImpactRiskAssessment {
public:
    static double calculate_kinetic_energy(double mass, double velocity);
};

class OrbitIntersection {
public:
    static bool check_orbit_intersection(double neo_orbit_params, double earth_orbit_params);
};

class SurfaceGravity {
public:
    static double calculate_surface_gravity(double mass, double radius);
};

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
        cout << "Miles: " << diameter["miles"]["estimated_diameter_min"] << " - "
             << diameter["miles"]["estimated_diameter_max"] << " miles" << endl;
        cout << "Feet: " << diameter["feet"]["estimated_diameter_min"] << " - "
             << diameter["feet"]["estimated_diameter_max"] << " feet" << endl;

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

// Placeholder for gravitational constant G (for surface gravity calculation)
const double G = 6.67430e-11;

// Callback function to capture the response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s) {
    size_t totalSize = size * nmemb;
    s->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Asking the user for date inputs
void ask_for_dates(string& startDate, string& endDate) {
    cout << "Enter the search start date (YYYY-MM-DD): ";
    cin >> startDate;
    cout << "Enter the search end date (YYYY-MM-DD): ";
    cin >> endDate;
}

bool load_from_file(json& jsonData, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open the file!" << endl;
        return false;
    }
    file >> jsonData;
    return true;
}

void process_neo_data(const json& jsonData) {
    try {
        auto& neo_objects = jsonData["near_earth_objects"];

        // List all available dates with NEO data
        vector<string> available_dates;
        cout << "\nAvailable dates with NEO data:\n";
        int i = 1;
        for (auto& el : neo_objects.items()) {
            string date = el.key();
            available_dates.push_back(date);  // Store available dates
            cout << i << ". " << date << endl;
            i++;
        }

        while (true) {
            cout << "\nEnter a date from the list above (YYYY-MM-DD), or type 'exit' to quit: ";
            string selected_date;
            cin >> selected_date;

            if (selected_date == "exit") {
                break;
            }

            // Check if the entered date is in the list of available dates
            if (find(available_dates.begin(), available_dates.end(), selected_date) != available_dates.end()) {
                // Get the NEOs for the selected date
                auto neos = neo_objects[selected_date];
                if (!neos.empty()) {
                    // If multiple NEOs exist, allow user to choose one
                    cout << "\nThere are " << neos.size() << " NEOs for the date " << selected_date << ".\n";
                    for (size_t j = 0; j < neos.size(); j++) {
                        cout << j + 1 << ". " << neos[j]["name"] << endl;
                    }

                    int neo_choice;
                    cout << "\nSelect a NEO by number: ";
                    cin >> neo_choice;

                    // Validate user choice
                    if (neo_choice > 0 && neo_choice <= neos.size()) {
                        // Output data for the selected NEO
                        output_neo_data(neos[neo_choice - 1]);  // Call the function to output NEO data
                    } else {
                        cout << "Invalid choice, please select a valid NEO number." << endl;
                    }
                } else {
                    cout << "No NEOs found for the selected date." << endl;
                }
            } else {
                cout << "Invalid date or no data available for the selected date." << endl;
            }
        }
    } catch (const exception& e) {
        cerr << "Error parsing data: " << e.what() << endl;
    }
}


int main() {
    CURL* curl;
    CURLcode res;
    string neo_data;
    string startDate, endDate;
    loadEnvFile(".env");

    // Get the start and end dates from the user
    cout << "\nWelcome to the NEO Analyzer!" << endl;
    ask_for_dates(startDate, endDate);

    // Initializing cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        // NASA API URL
        const string baseUrl = "https://api.nasa.gov/neo/rest/v1/feed";
        const char* apiKeyEnv = std::getenv("API_KEY");
        string apiKey = apiKeyEnv ? apiKeyEnv : "";
        if (apiKey.empty()) {
            cerr << "API key is missing. Please set the API_KEY environment variable." << endl;
            return 1;
        }

        // Constructing the full URL with user-specified dates
        string url = baseUrl + "?start_date=" + startDate + "&end_date=" + endDate + "&api_key=" + apiKey;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &neo_data);

        // Performing the request
        res = curl_easy_perform(curl);

        json jsonData;

        if(res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            cout << "Loading data from local json file..." << endl;
            if (load_from_file(jsonData, "data.json")) {
                process_neo_data(jsonData);
            }
        } else {
            try{
                jsonData = json::parse(neo_data);
                cout << "\nSuccessfully fetched data from NASA NEO API!" << endl;
                process_neo_data(jsonData);
            } catch (const exception& e) {
                cerr << "\nError parsing data: " << e.what() << endl;
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
