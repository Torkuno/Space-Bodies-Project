#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "src/get_data.h"
#include "src/planets.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>

using namespace std;
namespace fs = std::__fs::filesystem;
std::ofstream file;

// Constants for scaling and positioning
const double EARTH_RADIUS = 6371.0;
const double SCALE_FACTOR = 0.00001;
const float WINDOW_CENTER_X = 400;
const float WINDOW_CENTER_Y = 400;
float timeElapsed = 0.0f;
bool draggingSlider = false;

// Function to create and manage the slider
void drawSlider(sf::RenderWindow& window, sf::RectangleShape& slider, sf::RectangleShape& handle, float& timeElapsed) {
    // Event handling for dragging the slider
    if (draggingSlider) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        float newPosX = mousePos.x - handle.getSize().x / 2;
        if (newPosX >= slider.getPosition().x && newPosX <= slider.getPosition().x + slider.getSize().x - handle.getSize().x) {
            handle.setPosition(newPosX, handle.getPosition().y);
            // Update time based on slider position (normalized between 0 and 1)
            timeElapsed = (handle.getPosition().x - slider.getPosition().x) / (slider.getSize().x - handle.getSize().x);
        }
    }
    else{
        timeElapsed += 0.001;
        if (timeElapsed > 1) timeElapsed -= 1;
        sf::sleep(sf::milliseconds(10));
    }

    // Draw slider and handle
    window.draw(slider);
    window.draw(handle);
}

// SpaceBody Class
class SpaceBody {
public:
    SpaceBody(const string& name, double diameter, double mass)
        : name(name), diameter(diameter), mass(mass) {}

    virtual void printInfo() const {
        cout << "Name: " << name << ", Diameter: " << diameter << " km, Mass: " << mass << " kg" << endl;
    }

    double calculateSurfaceGravity() const {
        const double G = 6.67430e-11;
        double radius_m = (diameter * 1000) / 2.0;
        return (G * mass) / (radius_m * radius_m);
    }

    double calculateEscapeVelocity() const {
        const double G = 6.67430e-11;
        double radiusMeters = (diameter * 1000) / 2.0;
        double escapeVelocity_m_s = sqrt((2 * G * mass) / radiusMeters);
        return escapeVelocity_m_s / 1000.0;
    }

    double getMass() const { return mass; }
    double getDiameter() const { return diameter; }

    virtual ~SpaceBody() {
        cout << "Space body " << name << " memory freed." << endl;
    }

protected:
    string name;
    double diameter;
    double mass;
};

// Planet Class
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

    ~Planet() {
        cout << "Planet " << name << " memory freed." << endl;
    }
};

// Asteroid Class
class Asteroid : public SpaceBody {
public:
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

    Asteroid(const json& asteroidData)
        : SpaceBody(
            asteroidData["name"],
            asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>(),
            calculateMass(asteroidData)
          ),
          id(asteroidData["id"]),
          nasa_jpl_url(asteroidData["nasa_jpl_url"]),
          absolute_magnitude(asteroidData["absolute_magnitude_h"]),
          isDangerous(asteroidData["is_potentially_hazardous_asteroid"]) {
        auto diameter = asteroidData["estimated_diameter"];
        minDiameterKm = diameter["kilometers"]["estimated_diameter_min"].get<double>();
        maxDiameterKm = diameter["kilometers"]["estimated_diameter_max"].get<double>();

        auto close_approach = asteroidData["close_approach_data"][0];
        closeApproachDate = close_approach["close_approach_date"];
        relativeVelocityKmPerS = stod(close_approach["relative_velocity"]["kilometers_per_second"].get<string>());
        missDistanceKm = stod(close_approach["miss_distance"]["kilometers"].get<string>());

        missDistanceKm = std::max(missDistanceKm / 2.0, EARTH_RADIUS * 2);
        mass = calculateMass(asteroidData);
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

    double calculateImpactEnergy() const {
        double velocity_m_s = relativeVelocityKmPerS * 1000.0;
        double energy_joules = 0.5 * mass * pow(velocity_m_s, 2);
        double energy_megatons = energy_joules / 4.184e15;
        return energy_megatons;
    }

    Asteroid operator+(const Asteroid& other) const {
        Asteroid combinedAsteroid(*this);

        combinedAsteroid.name = name + " & " + other.name;
        combinedAsteroid.minDiameterKm += other.minDiameterKm;
        combinedAsteroid.maxDiameterKm += other.maxDiameterKm;
        combinedAsteroid.mass += other.mass;
        combinedAsteroid.relativeVelocityKmPerS += other.relativeVelocityKmPerS;
        combinedAsteroid.missDistanceKm += other.missDistanceKm;

        combinedAsteroid.isDangerous = ((combinedAsteroid.minDiameterKm > 280) || (combinedAsteroid.relativeVelocityKmPerS > 5.0));
        return combinedAsteroid;
    }

    ~Asteroid() {
        cout << "Asteroid " << name << " memory freed." << endl;
    }

private:
    string id;
    string nasa_jpl_url;
    double absolute_magnitude;
    double minDiameterKm;
    double maxDiameterKm;
    bool isDangerous;
    string closeApproachDate;
    double relativeVelocityKmPerS;
    double missDistanceKm;

    static double calculateMass(const json& asteroidData) {
        const double density = 3000.0;
        double diameterMin_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>() * 1000.0;
        double diameterMax_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_max"].get<double>() * 1000.0;

        double radiusMin = diameterMin_m / 2.0;
        double radiusMax = diameterMax_m / 2.0;

        double volumeMin = (4.0 / 3.0) * M_PI * pow(radiusMin, 3);
        double volumeMax = (4.0 / 3.0) * M_PI * pow(radiusMax, 3);

        double avgVolume = (volumeMin + volumeMax) / 2.0;

        double mass = density * avgVolume;
        return mass;
    }
};

void handlePlanetOptions(Asteroid& asteroid) {
    bool planetMenu = true;
    while (planetMenu) {
        cout << "\nPlease select an option:\n";
        cout << "1. Display information on a planet.\n";
        cout << "2. Estimate risk of damage if Asteroid Collides.\n";
        cout << "3. Visualize the asteroid's orbit.\n";
        cout << "4. Exit planet analysis.\n";
        cout << "Enter your choice: ";

        int planetChoice;
        cin >> planetChoice;

        switch (planetChoice) {
            case 1: {
                for (size_t i = 0; i < predefinedPlanets.size(); ++i) {
                    cout << i + 1 << ". " << predefinedPlanets[i].name << endl;
                }
                int planetSelection;
                cin >> planetSelection;

                if (planetSelection > 0 && planetSelection <= predefinedPlanets.size()) {
                    const auto& planetData = predefinedPlanets[planetSelection - 1];
                    Planet planet(planetData.name, planetData.diameter, planetData.mass);
                    planet.printInfo();
                } else {
                    cout << "Invalid selection.\n";
                }
                break;
            }
            case 3: {
                sf::RenderWindow window(sf::VideoMode(800, 800), "Asteroid Elliptical Orbit");

                // Load Earth texture
                sf::Texture earthTexture;
                if (!earthTexture.loadFromFile("Earth_Image.png")) {
                    cerr << "Error loading Earth texture." << endl;
                }
                sf::Sprite earthSprite;
                earthSprite.setTexture(earthTexture);
                earthSprite.setPosition(WINDOW_CENTER_X - earthSprite.getLocalBounds().width / 2,
                                        WINDOW_CENTER_Y - earthSprite.getLocalBounds().height / 2);

                // Slider for controlling the orbit position
                sf::RectangleShape slider(sf::Vector2f(300, 10));
                slider.setPosition(250, 700);
                slider.setFillColor(sf::Color::White);

                sf::RectangleShape handle(sf::Vector2f(20, 30));
                handle.setPosition(250, 690);
                handle.setFillColor(sf::Color::Green);

                // Ellipse orbit
                float semiMajorAxis = 200.0f;  // X-axis length
                float semiMinorAxis = 100.0f;  // Y-axis length

                sf::CircleShape asteroidShape(10);  // Represent the asteroid
                asteroidShape.setFillColor(sf::Color::Red);

                // Draw orbital line
                sf::CircleShape ellipse((int)semiMinorAxis);
                ellipse.setFillColor(sf::Color(0, 0, 0));


                ellipse.setScale(semiMajorAxis/semiMinorAxis, 1);

                ellipse.setPosition(WINDOW_CENTER_X - ellipse.getLocalBounds().width,
                                        WINDOW_CENTER_Y - ellipse.getLocalBounds().height / 2);

                ellipse.setOutlineThickness(3);
                ellipse.setOutlineColor(sf::Color(30, 30, 30));


                while (window.isOpen()) {
                    sf::Event event;
                    while (window.pollEvent(event)) {
                        if (event.type == sf::Event::Closed) window.close();
                        if (event.type == sf::Event::MouseButtonPressed) {
                            if (handle.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                draggingSlider = true;
                            }
                        }
                        if (event.type == sf::Event::MouseButtonReleased) {
                            draggingSlider = false;
                        }
                    }

                    // Clear window
                    window.clear();

                    window.draw(ellipse);

                    // Draw Earth
                    window.draw(earthSprite);

                    // Draw orbit (Ellipse logic)
                    float angle = 2 * M_PI * timeElapsed;  // Use timeElapsed to get the angle
                    float x = semiMajorAxis * cos(angle) + WINDOW_CENTER_X;
                    float y = semiMinorAxis * sin(angle) + WINDOW_CENTER_Y;
                    asteroidShape.setPosition(x - asteroidShape.getRadius(), y - asteroidShape.getRadius());

                    // Draw asteroid
                    window.draw(asteroidShape);

                    // Draw the slider and handle
                    drawSlider(window, slider, handle, timeElapsed);

                    window.display();
                }
                break;
            }
            case 4:
                planetMenu = false;
                break;
            default:
                cout << "Invalid choice.\n";
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

        const char* apiKeyEnv = getenv("API_KEY");
        string apiKey = apiKeyEnv ? apiKeyEnv : "";

        if (apiKey.empty()) {
            cerr << "API key is missing. Please set the API_KEY environment variable." << endl;
            return 1;
        }

        json jsonData;
        json selectedNeoJson;

        string neo_data = fetch_neo_data(selectedDate, apiKey);

        if (neo_data.empty()) {
            cerr << "Failed to fetch data from NASA API. Loading data from file..." << endl;

            if (!load_from_file(jsonData, "data.json")) {
                cerr << "Failed to load data from file." << endl;
                return 1;
            }

            selectedNeoJson = process_neo_data(jsonData, selectedDate);
        } else {
            try {
                jsonData = json::parse(neo_data);
                selectedNeoJson = process_neo_data(jsonData, selectedDate);
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
                            asteroid1.printInfo();
                            break;
                        case 2:
                            cout << "\nSurface Gravity: " << asteroid1.calculateSurfaceGravity() << " m/s^2\n";
                            break;
                        case 3:
                            cout << "\nImpact Energy: " << asteroid1.calculateImpactEnergy() << " megatons of TNT\n";
                            break;
                        case 4: {
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
                                    asteroid2.printInfo();

                                    Asteroid combinedAsteroid = asteroid1 + asteroid2;
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
                            handlePlanetOptions(asteroid1);
                            break;
                        case 6:
                            asteroidMenu = false;
                            break;
                        default:
                            cout << "Invalid choice.\n";
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