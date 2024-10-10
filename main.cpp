#include <iostream>
#include <string>
#include "src/get_data.h"
#include "src/planets.h"
#include "src/classes.h"
#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

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
