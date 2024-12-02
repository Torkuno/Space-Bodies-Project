#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "src/get_data.h"
#include "src/planets.h"  // Include the planets header
#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;
namespace fs = std::filesystem;
std::ofstream file;

// ** Class Definitions **

// Base class for Space Bodies (e.g., Planets, Asteroids)
class SpaceBody {
public:
    SpaceBody(const string& name, double diameter, double mass)
        : name(name), diameter(diameter), mass(mass) {}

    virtual void printInfo() const {
        cout << "Name: " << name << ", Diameter: " << diameter << " km, Mass: " << mass << " kg" << endl;
    }

    // Calculate surface gravity
    double calculateSurfaceGravity() const {
        const double G = 6.67430e-11;  // Gravitational constant (m^3 kg^-1 s^-2)
        double radius_m = (diameter * 1000) / 2.0;  // Convert km to meters and get radius
        return (G * mass) / (radius_m * radius_m);  // Surface gravity in m/s^2
    }

    // Calculate escape velocity
    double calculateEscapeVelocity() const {
        const double G = 6.67430e-11;  // Gravitational constant (m^3 kg^-1 s^-2)
        double radiusMeters = (diameter * 1000) / 2.0;  // Convert km to meters and get radius
        double escapeVelocity_m_s = sqrt((2 * G * mass) / radiusMeters);  // Escape velocity in m/s
        return escapeVelocity_m_s / 1000.0;  // Convert to km/s
    }

    // Getter for mass and diameter
    double getMass() const { return mass; }
    double getDiameter() const { return diameter; }

    // Destructor
    virtual ~SpaceBody() {
        cout << "Space body " << name << " memory freed." << endl;
    }

protected:
    string name;
    double diameter;  // in kilometers
    double mass;      // in kilograms
};

// Derived class for Planets
class Planet : public SpaceBody {
public:
    Planet(const string& name, double diameter, double mass)
        : SpaceBody(name, diameter, mass) {}

    void printInfo() const override {
        cout << "Planet Name: " << name << ", Mass: " << mass << " kg, Diameter: " << diameter << " km" << endl;
        cout << "Surface Gravity: " << calculateSurfaceGravity() << " m/s^2" << endl;
        cout << "Escape Velocity: " << calculateEscapeVelocity() << " km/s" << endl;
        // add infos to CSV file
        file << "," << name << ", , ," << diameter << ", , , , , ," << mass << "," << calculateSurfaceGravity() << ", ," << calculateEscapeVelocity() << "\n";
    }

    // Destructor
    ~Planet() {
        cout << "Planet " << name << " memory freed." << endl;
    }
};

// Derived class for Asteroids
class Asteroid : public SpaceBody {
public:
    // Copy constructor
    Asteroid(const Asteroid& other)
        : SpaceBody(other.name, other.diameter, other.mass),
          id(other.id),
          nasa_jpl_url(other.nasa_jpl_url),
          absolute_magnitude(other.absolute_magnitude),
          minDiameterKm(other.minDiameterKm),
          maxDiameterKm(other.maxDiameterKm),
          isDangerous(other.isDangerous),
          closeApproachDate(other.closeApproachDate),
          relativeVelocityKmPerS(other.relativeVelocityKmPerS),
          missDistanceKm(other.missDistanceKm) {
        cout << "Asteroid " << name << " copied." << endl;
    }

    // Asteroid constructor, extracts data from the JSON object
    Asteroid(const json& asteroidData)
        : SpaceBody(
            asteroidData["name"],
            asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>(),
            calculateMass(asteroidData)
          ),
          id(asteroidData["id"]),
          nasa_jpl_url(asteroidData["nasa_jpl_url"]),
          absolute_magnitude(asteroidData["absolute_magnitude_h"]),
          isDangerous(asteroidData["is_potentially_hazardous_asteroid"])
    {
        // Extract diameter information
        auto diameter = asteroidData["estimated_diameter"];
        minDiameterKm = diameter["kilometers"]["estimated_diameter_min"].get<double>();
        maxDiameterKm = diameter["kilometers"]["estimated_diameter_max"].get<double>();

        // Extract close approach data
        auto close_approach = asteroidData["close_approach_data"][0];
        closeApproachDate = close_approach["close_approach_date"];
        relativeVelocityKmPerS = stod(close_approach["relative_velocity"]["kilometers_per_second"].get<string>());
        missDistanceKm = stod(close_approach["miss_distance"]["kilometers"].get<string>());
    }

    void printInfo() const override {
        cout << "Asteroid ID: " << id << endl;
        cout << "Name: " << name << endl;
        cout << "NASA JPL URL: " << nasa_jpl_url << endl;
        cout << "Absolute Magnitude (H): " << absolute_magnitude << endl;
        cout << "Diameter (Min): " << minDiameterKm << " km, Max: " << maxDiameterKm << " km" << endl;
        cout << "Is Potentially Hazardous: " << (isDangerous ? "Yes" : "No") << endl;
        cout << "Close Approach Date: " << closeApproachDate << endl;
        cout << "Relative Velocity: " << relativeVelocityKmPerS << " km/s" << endl;
        cout << "Miss Distance: " << missDistanceKm << " km" << endl;
        cout << "Mass: " << mass << " kg" << endl;
        cout << "Surface Gravity: " << calculateSurfaceGravity() << " m/s^2" << endl;
        cout << "Impact Energy: " << calculateImpactEnergy() << " megatons of TNT" << endl;
        // add infos to CSV file
        file << id << "," << name << "," << nasa_jpl_url << "," << absolute_magnitude << "," << minDiameterKm << "," << maxDiameterKm << "," << isDangerous << "," << closeApproachDate << "," << relativeVelocityKmPerS << "," << missDistanceKm << "," << mass << "," << calculateSurfaceGravity() << "," << calculateImpactEnergy() << "\n";
    }

    // Calculate impact energy in megatons of TNT
    double calculateImpactEnergy() const {
        // Kinetic energy formula: E = 0.5 * mass * velocity^2
        double velocity_m_s = relativeVelocityKmPerS * 1000.0;  // Convert km/s to m/s
        double energy_joules = 0.5 * mass * pow(velocity_m_s, 2);  // Energy in Joules

        // Convert Joules to megatons of TNT (1 megaton TNT = 4.184e15 J)
        double energy_megatons = energy_joules / 4.184e15;

        return energy_megatons;
    }

    // Operator overload for adding two Asteroids
    Asteroid operator+(const Asteroid& other) const {
        // Create a copy of the current asteroid
        Asteroid combinedAsteroid(*this);

        // Modify necessary fields
        combinedAsteroid.name = name + " & " + other.name; // Combine names
        // Combine diameters
        combinedAsteroid.minDiameterKm += other.minDiameterKm;
        combinedAsteroid.maxDiameterKm += other.maxDiameterKm;
        // Combine masses
        combinedAsteroid.mass += other.mass;
        // Combine relative velocities
        combinedAsteroid.relativeVelocityKmPerS += other.relativeVelocityKmPerS;
        // Combine miss distances
        combinedAsteroid.missDistanceKm += other.missDistanceKm;
        // Update the potentially hazardous status
        combinedAsteroid.isDangerous =
            ((combinedAsteroid.minDiameterKm > 280) || (combinedAsteroid.relativeVelocityKmPerS > 5.0));

        return combinedAsteroid;
    }

    // Destructor
    ~Asteroid() {
        cout << "Asteroid " << name << " memory freed." << endl;
    }

private:
    // Member variables for the asteroid
    string id;
    string nasa_jpl_url;
    double absolute_magnitude;
    double minDiameterKm;
    double maxDiameterKm;
    bool isDangerous;
    string closeApproachDate;
    double relativeVelocityKmPerS;
    double missDistanceKm;

    // Static helper function to calculate mass
    static double calculateMass(const json& asteroidData) {
        // Assume density of asteroid (in kg/m^3)
        const double density = 3000.0;  // Example density in kg/m^3

        double diameterMin_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>() * 1000.0; // km to m
        double diameterMax_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_max"].get<double>() * 1000.0; // km to m

        double radiusMin = diameterMin_m / 2.0;
        double radiusMax = diameterMax_m / 2.0;

        double volumeMin = (4.0 / 3.0) * M_PI * pow(radiusMin, 3);
        double volumeMax = (4.0 / 3.0) * M_PI * pow(radiusMax, 3);

        double avgVolume = (volumeMin + volumeMax) / 2.0;

        double mass = density * avgVolume;  // mass in kg

        return mass;
    }
};

void handlePlanetOptions(Asteroid& asteroid) {
    bool planetMenu = true;
    while (planetMenu) {
        std::cout << "\nPlease select an option:\n";
        std::cout << "1. Display information on a planet.\n";
        std::cout << "2. Estimate risk of damage if Asteroid Collides.\n";
        std::cout << "3. Exit planet analysis.\n";
        std::cout << "Enter your choice: ";

        int planetChoice;
        std::cin >> planetChoice;

        switch (planetChoice) {
            case 1: {
                // Display a dropdown of planets
                for (size_t i = 0; i < predefinedPlanets.size(); ++i) {
                    std::cout << i + 1 << ". " << predefinedPlanets[i].name << std::endl;
                }
                std::cout << "\nSelect a planet to display information:\n";
                int planetSelection;
                std::cin >> planetSelection;

                if (planetSelection > 0 && planetSelection <= predefinedPlanets.size()) {
                    // Use the Planet class to display planet information
                    const auto& planetData = predefinedPlanets[planetSelection - 1];
                    Planet planet(planetData.name, planetData.diameter, planetData.mass);
                    planet.printInfo();
                } else {
                    std::cout << "Invalid selection.\n";
                }
                break;
            }
            case 2: {
                // Combine asteroid with a planet
                std::cout << "\nSelect a planet to combine with the asteroid:\n";
                for (size_t i = 0; i < predefinedPlanets.size(); ++i) {
                    std::cout << i + 1 << ". " << predefinedPlanets[i].name << std::endl;
                }
                int planetSelection;
                std::cin >> planetSelection;

                if (planetSelection > 0 && planetSelection <= predefinedPlanets.size()) {
                    const auto& planetData = predefinedPlanets[planetSelection - 1];
                    Planet planet(planetData.name, planetData.diameter, planetData.mass);
                    std::cout << "\nPlanet Name: " << planetData.name << "\n";

                    // Calculate asteroid impact energy and compare with the planet's mass and escape velocity
                    double asteroidImpactEnergy = asteroid.calculateImpactEnergy();
                    double planetEscapeVelocity = planet.calculateEscapeVelocity();

                    if (asteroidImpactEnergy > 0.01 && asteroid.getMass() > (planet.getMass() * 0.00001)) {
                        std::cout << "\nWarning: The asteroid could cause significant damage to " << planetData.name << "!\n";
                    } else {
                        std::cout << "\nThe asteroid will likely not cause significant damage to " << planetData.name << ".\n";
                    }
                } else {
                    std::cout << "Invalid selection.\n";
                }
                break;
            }
            case 3:
                planetMenu = false;
                break;
            default:
                std::cout << "Invalid choice. Please select a valid option.\n";
        }
    }
}


int main() {
    loadEnvFile(".env");

    // open CSV file for store user info he discovered during execution:
    std::string nextCsvName = "user_discovered.csv";
    file.open(nextCsvName);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << nextCsvName << std::endl;
        return 1;
    }
    // Write the column headers to the file
    file << "Asteroid ID,"
    "Name,"
    "NASA JPL URL,"
    "Absolute Magnitude (H),"
    "Min Diameter,"
    "Max Diameter,"
    "Is Potentially Hazardous,"
    "Close Approach Date (YYYY-MM-DD),"
    "Relative Velocity (km/s),"
    "Miss Distance (km),"
    "Mass (kg),"
    "Surface Gravity (m/s^2),"
    "Impact Energy (TNT),"
    "Escape Velocity (km/s),"
    "\n\n\n";

    bool continueAnalyzing = true;
    while (continueAnalyzing) {
        string selectedDate;
        cout << "\n\nWelcome to the NEO Analyzer!" << endl;
        cout << "Enter a date (YYYY-MM-DD) to search for NEOs: ";
        cin >> selectedDate;

        const char* apiKeyEnv = getenv("API_KEY"); // Get API key environment variable
        string apiKey = apiKeyEnv ? apiKeyEnv : "";  // If missing, use an empty string

        if (apiKey.empty()) {
            cerr << "API key is missing. Please set the API_KEY environment variable." << endl;
            return 1;  // Exit the program if API key is not set
        }

        // Declare jsonData and selected NEOJson outside the if-else blocks
        json jsonData;          // JSON object to store loaded data
        json selectedNeoJson;   // To hold the selected NEO data

        // Fetch NEO data for the selected date and the API key
        string neo_data = fetch_neo_data(selectedDate, apiKey);

        if (neo_data.empty()) {
            cerr << "Failed to fetch data from NASA API. Loading data from file..." << endl;

            // If fetching from API fails, load from file
            if (!load_from_file(jsonData, "data.json")) {
                cerr << "Failed to load data from file." << endl;
                return 1; // Exit if both API and file loading fail
            }

            selectedNeoJson = process_neo_data(jsonData, selectedDate); // process data from file
        } else {
            try {
                jsonData = json::parse(neo_data);
                selectedNeoJson = process_neo_data(jsonData, selectedDate); // process the data fetched from Api
            } catch (const exception& e) {
                cerr << "Error parsing data: " << e.what() << endl;
                return 1;
            }
        }

        if (!selectedNeoJson.empty()) {
            try {
                Asteroid asteroid1(selectedNeoJson);

                bool asteroidMenu = true;
                while (asteroidMenu) {
                    cout << "\nPlease select an option:\n";
                    cout << "1. Print all information about the asteroid.\n";
                    cout << "2. Calculate and display the surface gravity.\n";
                    cout << "3. Calculate and display the impact energy.\n";
                    cout << "4. Combine this asteroid with another asteroid.\n";
                    cout << "5. Analyze planets in the solar system.\n";
                    cout << "6. Exit or Return to main menu.\n";
                    cout << "Enter your choice: ";

                    int choice;
                    cin >> choice;

                    switch (choice) {
                        case 1:
                            cout << "\n--- Asteroid Information ---\n";
                            asteroid1.printInfo();
                            break;
                        case 2:
                            cout << "\nSurface Gravity: " << asteroid1.calculateSurfaceGravity() << " m/s^2\n";
                            break;
                        case 3:
                            cout << "\nImpact Energy: " << asteroid1.calculateImpactEnergy() << " megatons of TNT\n";
                            break;
                        case 4: {
                            cout << "\n--- Asteroid Information ---\n";
                            asteroid1.printInfo();
                            string selectedDate2;
                            cout << "\nEnter a second date (YYYY-MM-DD) to search for NEOs: ";
                            cin >> selectedDate2;
                            string neo_data2 = fetch_neo_data(selectedDate2, apiKey);

                            if (!neo_data2.empty()) {
                                jsonData = json::parse(neo_data2);
                                json selectedNeoJson2 = process_neo_data(jsonData, selectedDate2);

                                if (!selectedNeoJson2.empty()) {
                                    Asteroid asteroid2(selectedNeoJson2);
                                    cout << "\n--- Second Asteroid Information ---\n";
                                    asteroid2.printInfo();

                                    cout << "\nCombining the two asteroids...\n";
                                    Asteroid combinedAsteroid = asteroid1 + asteroid2;
                                    cout << "\n--- Combined Asteroid Information ---\n";
                                    combinedAsteroid.printInfo();
                                } else {
                                    cout << "No asteroid selected for the second date.\n";
                                }
                            } else {
                                cout << "Failed to fetch data for the second date from NASA API.\n";
                            }
                            break;
                        }
                        case 5:
                            handlePlanetOptions(asteroid1);  // Move to planet analysis
                            break;
                        case 6:
                            asteroidMenu = false;
                            break;
                        default:
                            cout << "Invalid choice. Please select a valid option.\n";
                    }

                    if (asteroidMenu) {
                        cout << "\nDo you want to perform another action on this asteroid? (y/n): ";
                        char continueChoice;
                        cin >> continueChoice;
                        if (continueChoice == 'n' || continueChoice == 'N') {
                            asteroidMenu = false;
                        }
                    }
                }

            } catch (const exception& e) {
                cerr << "Error creating Asteroid object: " << e.what() << endl;
            }
        } else {
            cout << "No asteroid selected.\n";
        }

        cout << "\nDo you want to analyze another asteroid? (y/n): ";
        char mainChoice;
        cin >> mainChoice;
        if (mainChoice == 'n' || mainChoice == 'N') {
            continueAnalyzing = false;
            cout << "Exiting the NEO Analyzer. Goodbye!\n";
        }
    }

    return 0;
}
