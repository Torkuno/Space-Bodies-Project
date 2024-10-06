#include <iostream>
#include <string>
#include "src/get_data.h"

using namespace std;



//classes
class SpaceBody {
public:
    SpaceBody(const std::string& name, double diameter) : name(name), diameter(diameter) {}

    virtual void printInfo() {
        std::cout << "Name: " << name << ", diameter: " << diameter << std::endl;
    }

    // Destructor
    virtual ~SpaceBody() {
        std::cout << "Space body " << name << " destroyed." << std::endl;
    }

protected:
    std::string name;
    double diameter;
};

class Planet : public SpaceBody {
public:
    Planet(const std::string& name, double diameter, double mass) : SpaceBody(name, diameter), mass(mass) {}

    void printInfo() override {
        std::cout << "Name: " << name << ", Mass: " << mass << ", diameter: " << diameter << std::endl;
    }

    // Destructor
    ~Planet() {
        std::cout << "Planet " << name << " destroyed." << std::endl;
    }

private:
    double mass;
};

class Asteroid : public SpaceBody {
public:
    // copy constructor
    Asteroid(const Asteroid& other) : SpaceBody(other.name, other.diameter),
        id(other.id),
        nasa_jpl_url(other.nasa_jpl_url),
        absolute_magnitude(other.absolute_magnitude),
        minDiameterKm(other.minDiameterKm),
        maxDiameterKm(other.maxDiameterKm),
        isDangerous(other.isDangerous),
        closeApproachDate(other.closeApproachDate),
        relativeVelocityKmPerS(other.relativeVelocityKmPerS),
        missDistanceKm(other.missDistanceKm), mass(other.mass) {
        std::cout << "Asteroid " << name << " copied." << std::endl;
        }
    // Asteroid constructor, extracts data from the JSON object
    Asteroid(const nlohmann::json& asteroidData) : SpaceBody(asteroidData["name"], asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"]),
          id(asteroidData["id"]),
          nasa_jpl_url(asteroidData["nasa_jpl_url"]),
          absolute_magnitude(asteroidData["absolute_magnitude_h"]),
          isDangerous(asteroidData["is_potentially_hazardous_asteroid"])
    {
        // Extract diameter information
        auto diameter = asteroidData["estimated_diameter"];
        minDiameterKm = diameter["kilometers"]["estimated_diameter_min"];
        maxDiameterKm = diameter["kilometers"]["estimated_diameter_max"];

        // Extract close approach data
        auto close_approach = asteroidData["close_approach_data"][0];
        closeApproachDate = close_approach["close_approach_date"];
        relativeVelocityKmPerS = std::stod(close_approach["relative_velocity"]["kilometers_per_second"].get<std::string>());
        missDistanceKm = std::stod(close_approach["miss_distance"]["kilometers"].get<std::string>());

        // calculate approx mass
        mass = calculateMass();
    }

    void printInfo() override {
        std::cout << "Asteroid ID: " << id << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "NASA JPL URL: " << nasa_jpl_url << std::endl;
        std::cout << "Absolute Magnitude (H): " << absolute_magnitude << std::endl;
        std::cout << "Diameter (Min): " << minDiameterKm << " km, Max: " << maxDiameterKm << " km" << std::endl;
        std::cout << "Is Potentially Hazardous: " << (isDangerous ? "Yes" : "No") << std::endl;
        std::cout << "Close Approach Date: " << closeApproachDate << std::endl;
        std::cout << "Relative Velocity: " << relativeVelocityKmPerS << " km/s" << std::endl;
        std::cout << "Miss Distance: " << missDistanceKm << " km" << std::endl;
        std::cout << "Mass: " << mass << " g" << std::endl;
    }

    // Operator overload for adding two Asteroids
    // add two asteroids to see if now they are dangerous
    Asteroid operator+(const Asteroid& other) const {
        // Create a copy of the current asteroid
        Asteroid combinedAsteroid(*this);

        // Modify necessary fields
        combinedAsteroid.name = name + " & " + other.name; // Combine names
        // Combine diameters
        combinedAsteroid.minDiameterKm = minDiameterKm + other.minDiameterKm;
        combinedAsteroid.maxDiameterKm = maxDiameterKm + other.maxDiameterKm;
        // Combine masses
        combinedAsteroid.mass = mass + other.mass;
        // Combine relative velocities
        combinedAsteroid.relativeVelocityKmPerS = relativeVelocityKmPerS + other.relativeVelocityKmPerS;
        // Combine miss distances
        combinedAsteroid.missDistanceKm = missDistanceKm + other.missDistanceKm;
        // Update the potentially hazardous status
        combinedAsteroid.isDangerous =
            ((combinedAsteroid.minDiameterKm > 280) || (combinedAsteroid.relativeVelocityKmPerS > 5.0));

        // Return the combined asteroid
        return combinedAsteroid;
    }

    // Destructor
    ~Asteroid() {
        std::cout << "Asteroid " << name << " destroyed." << std::endl;
    }

private:
    // New member variables for the asteroid
    std::string id;
    std::string nasa_jpl_url;
    double absolute_magnitude;
    double minDiameterKm;
    double maxDiameterKm;
    bool isDangerous;
    std::string closeApproachDate;
    double relativeVelocityKmPerS; //std::string
    double missDistanceKm;         //std::string
    double mass;
    double calculateMass() const {
        return (minDiameterKm + maxDiameterKm) * 1.0; // Placeholder calculation
    }
};









int main() {
    loadEnvFile(".env");

    // Ask the user for a single date
    string selectedDate;
    cout << "\n\nWelcome to the NEO Analyzer!" << endl;
    cout << "Enter a date (YYYY-MM-DD) to search for NEOs: ";
    cin >> selectedDate;

    const char* apiKeyEnv = std::getenv("API_KEY");  // Get API key from environment variable
    string apiKey = apiKeyEnv ? apiKeyEnv : "";      // If environment variable is missing, use empty string

    if (apiKey.empty()) {
        cerr << "API key is missing. Please set the API_KEY environment variable." << endl;
        return 1;  // Exit program if API key is not set
    }

    // Declare jsonData and selectedNeoJson outside the if-else blocks
    nlohmann::json jsonData;  // JSON object to store loaded data
    nlohmann::json selectedNeoJson; // To hold the selected NEO data

    // Fetch NEO data for the selected date and the API key
    string neo_data = fetch_neo_data(selectedDate, apiKey);

    if (neo_data.empty()) {
        cerr << "Failed to fetch data from NASA API. Loading data from file..." << endl;

        // If fetching from API fails, load from file
        if (!load_from_file(jsonData, "data.json")) {
            cerr << "Failed to load data from file." << endl;
            return 1;  // Exit if both API and file loading fail
        }

        selectedNeoJson = process_neo_data(jsonData, selectedDate);  // Process data from file

    } else {
        try {
            jsonData = nlohmann::json::parse(neo_data);  // Parse the JSON response
            selectedNeoJson = process_neo_data(jsonData, selectedDate);  // Process the data fetched from API

            /*
            //debugging
            std::cout << "Asteroid JSON Data: " << selectedNeoJson.dump(4) << std::endl;
            if (!selectedNeoJson["estimated_diameter"]["kilometers"]["estimated_diameter_min"].is_number()) {
                std::cerr << "Error: estimated_diameter_min is not a number." << std::endl;
            }
            */

            // Create asteroid class object
            Asteroid asteroid1(selectedNeoJson);
            cout << "\n\n--------------" << endl;
            asteroid1.printInfo();

            // Create another asteroid object, ask user for data
            string selectedDate2;
            cout << "\nEnter a date (YYYY-MM-DD) to search for NEOs: ";
            cin >> selectedDate2;
            string neo_data2 = fetch_neo_data(selectedDate2, apiKey);
            jsonData = nlohmann::json::parse(neo_data2);
            selectedNeoJson = process_neo_data(jsonData, selectedDate2);
            Asteroid asteroid2(selectedNeoJson);
            cout << "\n\n--------------" << endl;
            asteroid2.printInfo();

            // Add the two asteroids
            cout << "\n\n" << endl;
            Asteroid combinedAsteroid = asteroid1 + asteroid2;
            cout << "\n\n------combine asteroid--------\n" << endl;
            combinedAsteroid.printInfo();
            cout << "\n\n------destructor--------\n" << endl;

        } catch (const exception& e) {
            cerr << "Error parsing data: " << e.what() << endl;
            return 1;
        }
    }

    return 0;
}