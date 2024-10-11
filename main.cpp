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
#include "json.hpp"

using namespace std;

// Constants for scaling and positioning
const double EARTH_RADIUS = 6371.0;  // Earth radius in kilometers
const double SCALE_FACTOR = 0.00001;  // Reduced scaling factor for visualization
const float WINDOW_CENTER_X = 400;
const float WINDOW_CENTER_Y = 400;
float timeElapsed = 0.0f;  // Time-scrolling variable
bool draggingSlider = false;  // Slider drag state

// SpaceBody Class
class SpaceBody {
public:
    SpaceBody(const string& name, double diameter, double mass)
        : name(name), diameter(diameter), mass(mass) {}

    virtual void printInfo() const {
        cout << "Name: " << name << ", Diameter: " << diameter << " km, Mass: " << mass << " kg" << endl;
    }

    // Calculate surface gravity
    double calculateSurfaceGravity() const {
        const double G = 6.67430e-11;
        double radius_m = (diameter * 1000) / 2.0;
        return (G * mass) / (radius_m * radius_m);
    }

    // Calculate escape velocity
    double calculateEscapeVelocity() const {
        const double G = 6.67430e-11;
        double radiusMeters = (diameter * 1000) / 2.0;
        double escapeVelocity_m_s = sqrt((2 * G * mass) / radiusMeters);
        return escapeVelocity_m_s / 1000.0;
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
    double diameter;  
    double mass;      
};

// Derived class for Asteroids
class Asteroid : public SpaceBody {
public:
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

        // Adjusted miss distance for visualization
        missDistanceKm = std::max(missDistanceKm / 2.0, EARTH_RADIUS * 2);
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
        cout << "Surface Gravity: " << calculateSurfaceGravity() << " m/s^2" << endl;
    }

    // Calculate impact energy in megatons of TNT
    double calculateImpactEnergy() const {
        double velocity_m_s = relativeVelocityKmPerS * 1000.0;
        double energy_joules = 0.5 * mass * pow(velocity_m_s, 2);
        double energy_megatons = energy_joules / 4.184e15;
        return energy_megatons;
    }

    // Destructor
    ~Asteroid() {
        cout << "Asteroid " << name << " memory freed." << endl;
    }

    double getRelativeVelocityKmPerS() const {
        return relativeVelocityKmPerS;
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

    // Helper function to calculate mass
    static double calculateMass(const json& asteroidData) {
        const double density = 3000.0;
        double diameterMin_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>() * 1000.0;
        double radiusMin = diameterMin_m / 2.0;
        double volumeMin = (4.0 / 3.0) * M_PI * pow(radiusMin, 3);
        return density * volumeMin;
    }
};

// Orbit visualization using SFML, includes Earth image
void visualizeAsteroidOrbit(const Asteroid& asteroid) {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Asteroid Orbit Visualization");

    // Load Earth texture
    sf::Texture earthTexture;
    if (!earthTexture.loadFromFile("Earth_Image.jpeg")) {
        cerr << "Failed to load Earth image!" << endl;
        return;
    }

    sf::Sprite earthSprite;
    earthSprite.setTexture(earthTexture);
    earthSprite.setOrigin(earthTexture.getSize().x / 2, earthTexture.getSize().y / 2);
    earthSprite.setPosition(WINDOW_CENTER_X, WINDOW_CENTER_Y);
    earthSprite.setScale(0.2f, 0.2f);

    // Create orbit circle
    sf::CircleShape orbit;
    orbit.setRadius(200);
    orbit.setOrigin(orbit.getRadius(), orbit.getRadius());
    orbit.setPosition(WINDOW_CENTER_X, WINDOW_CENTER_Y);
    orbit.setOutlineColor(sf::Color::White);
    orbit.setOutlineThickness(2);
    orbit.setFillColor(sf::Color::Transparent);

    // Create asteroid shape
    sf::CircleShape asteroidShape;
    asteroidShape.setRadius(10);
    asteroidShape.setFillColor(sf::Color::Green);

    float angle = 0.0f;
    float speed = asteroid.getRelativeVelocityKmPerS() * 0.05f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        // Update asteroid position along the orbit
        angle += speed;
        float x = orbit.getPosition().x + orbit.getRadius() * std::cos(angle);
        float y = orbit.getPosition().y + orbit.getRadius() * std::sin(angle);
        asteroidShape.setPosition(x - asteroidShape.getRadius(), y - asteroidShape.getRadius());

        window.draw(earthSprite);
        window.draw(orbit);
        window.draw(asteroidShape);
        window.display();
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
            cerr << "Failed to fetch data from NASA API." << endl;
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
                Asteroid asteroid(selectedNeoJson);

                bool asteroidMenu = true;
                while (asteroidMenu) {
                    cout << "\nPlease select an option:\n";
                    cout << "1. Print all information about the asteroid.\n";
                    cout << "2. Visualize the asteroid's orbit.\n";
                    cout << "3. Exit.\n";
                    cout << "Enter your choice: ";

                    int choice;
                    cin >> choice;

                    switch (choice) {
                        case 1:
                            asteroid.printInfo();
                            break;
                        case 2:
                            visualizeAsteroidOrbit(asteroid);
                            break;
                        case 3:
                            asteroidMenu = false;
                            break;
                        default:
                            cout << "Invalid choice.\n";
                    }
                }
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
            }
        }

        cout << "\nDo you want to analyze another asteroid? (y/n): ";
        char mainChoice;
        cin >> mainChoice;
        if (mainChoice == 'n' || mainChoice == 'N') {
            continueAnalyzing = false;
            cout << "Exiting NEO Analyzer. Goodbye!\n";
        }
    }

    return 0;
}