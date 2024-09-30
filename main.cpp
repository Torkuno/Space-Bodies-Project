#include <iostream>
#include <curl/curl.h>
#include <string>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

// Placeholder for gravitational constant G (for surface gravity calculation)
const double G = 6.67430e-11;

// Callback function to capture the response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Asking the user for date inputs
void ask_for_dates(std::string& startDate, std::string& endDate) {
    std::cout << "Enter the search start date (YYYY-MM-DD): ";
    std::cin >> startDate;
    std::cout << "Enter the search end date (YYYY-MM-DD): ";
    std::cin >> endDate;
}

bool load_from_file(json& jsonData, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return false;
    }
    file >> jsonData;
    return true;
}

void process_neo_data(const json& jsonData) {
    try {
        std::cout << jsonData["near_earth_objects"]["2024-09-30"][0]["id"] << std::endl;
        while (true){
            std::cout << "\nSelect a date (YYYY-MM-DD): " << std::endl;
            int i=1;
            auto& neo_objects = jsonData["near_earth_objects"];
            for(auto& el : neo_objects.items()){
                std::string date = el.key();
                std::cout << i << ". " << date << std::endl;
                i++;
            }
            std::cout << "(Type 'exit' to exit)" << std::endl;
            std::string selected_date;
            std::cin >> selected_date;
            if (selected_date == "exit") {
                break;
            }
        }
    } catch (const std::exception& e){
        std::cerr << "Error parsing data: " << e.what() << std::endl;
    }
}

int main() {
    CURL* curl;
    CURLcode res;
    std::string neo_data;
    std::string startDate, endDate;

    // Get the start and end dates from the user
    std::cout << "Welcome to the NEO Analyzer!" << std::endl;
    ask_for_dates(startDate, endDate);

    // Initializing cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        // NASA API URL
        std::string baseUrl = "https://api.nasa.gov/neo/rest/v1/feed";
        std::string apiKey = "KEY_HERE";

        // Constructing the full URL with user-specified dates
        std::string url = baseUrl + "?start_date=" + startDate + "&end_date=" + endDate + "&api_key=" + apiKey;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &neo_data);

        // Performing the request
        res = curl_easy_perform(curl);

        json jsonData;

        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            std::cout << "Loading data from local json file..." << std::endl;
            if (load_from_file(jsonData, "data.json")) {
                process_neo_data(jsonData);
            }
        } else {
            try{
                jsonData = json::parse(neo_data);
                std::cout << "\nSuccessfully fetched data from NASA NEO API!" << std::endl;
                process_neo_data(jsonData);
            } catch (const std::exception& e) {
                std::cerr << "\nError parsing data: " << e.what() << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}