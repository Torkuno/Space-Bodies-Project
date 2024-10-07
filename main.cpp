#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include "src/get_data.h"

using namespace std;

// Classes
class SpaceBody {
public:
    SpaceBody(const string& name, double diameter) : name(name), diameter(diameter) {}

    virtual void printInfo() {
        cout << "Name: " << name << ", diameter: " << diameter << endl;
    }

    // Destructor
    virtual ~SpaceBody() {
        cout << "Space body " << name << " destroyed." << endl;
    }

protected:
    string name;
    double diameter;
};

class Planet : public SpaceBody {
public:
    Planet(const string& name, double diameter, double mass) : SpaceBody(name, diameter), mass(mass) {}

    void printInfo() override {
        cout << "Name: " << name << ", Mass: " << mass << ", diameter: " << diameter << endl;
    }

    // Destructor
    ~Planet() {
        cout << "Planet " << name << " destroyed." << endl;
    }

private:
    double mass;
};

class Asteroid : public SpaceBody {
public:
    // Copy constructor
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
        cout << "Asteroid " << name << " copied." << endl;
    }

    // Asteroid constructor, extracts data from the JSON object
    Asteroid(const json& asteroidData) : SpaceBody(asteroidData["name"], asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"]),
        id(asteroidData["id"]),
        nasa_jpl_url(asteroidData["nasa_jpl_url"]),
        absolute_magnitude(asteroidData["absolute_magnitude_h"]),
        isDangerous(asteroidData["is_potentially_hazardous_asteroid"]) {
        // Extract diameter information
        auto diameter = asteroidData["estimated_diameter"];
        minDiameterKm = diameter["kilometers"]["estimated_diameter_min"];
        maxDiameterKm = diameter["kilometers"]["estimated_diameter_max"];

        // Extract close approach data
        auto close_approach = asteroidData["close_approach_data"][0];
        closeApproachDate = close_approach["close_approach_date"];
        relativeVelocityKmPerS = stod(close_approach["relative_velocity"]["kilometers_per_second"].get<string>());
        missDistanceKm = stod(close_approach["miss_distance"]["kilometers"].get<string>());

        // Calculate approx mass
        mass = calculateMass();
    }

    // Print asteroid info
    void printInfo() override {
        cout << "Asteroid ID: " << id << endl;
        cout << "Name: " << name << endl;
        cout << "NASA JPL URL: " << nasa_jpl_url << endl;
        cout << "Absolute Magnitude (H): " << absolute_magnitude << endl;
        cout << "Diameter (Min): " << minDiameterKm << " km, Max: " << maxDiameterKm << " km" << endl;
        cout << "Is Potentially Hazardous: " << (isDangerous ? "Yes" : "No") << endl;
        cout << "Close Approach Date: " << closeApproachDate << endl;
        cout << "Relative Velocity: " << relativeVelocityKmPerS << " km/s" << endl;
        cout << "Miss Distance: " << missDistanceKm << " km" << endl;
        cout << "Mass: " << mass << " g" << endl;
    }

    // Operator overload for adding two Asteroids
    Asteroid operator+(const Asteroid& other) const {
        // Create a copy of the current asteroid
        Asteroid combinedAsteroid(*this);

        // Modify necessary fields
        combinedAsteroid.name = name + " & " + other.name; // Combine names
        combinedAsteroid.minDiameterKm = minDiameterKm + other.minDiameterKm; // Combine diameters
        combinedAsteroid.maxDiameterKm = maxDiameterKm + other.maxDiameterKm;
        combinedAsteroid.mass = mass + other.mass; // Combine masses
        combinedAsteroid.relativeVelocityKmPerS = relativeVelocityKmPerS + other.relativeVelocityKmPerS; // Combine relative velocities
        combinedAsteroid.missDistanceKm = missDistanceKm + other.missDistanceKm; // Combine miss distances
        combinedAsteroid.isDangerous = ((combinedAsteroid.minDiameterKm > 280) || (combinedAsteroid.relativeVelocityKmPerS > 5.0)); // Update the potentially hazardous status

        // Return the combined asteroid
        return combinedAsteroid;
    }

    // Destructor
    ~Asteroid() {
        cout << "Asteroid " << name << " destroyed." << endl;
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
    double mass;

    double calculateMass() const {
        const double density = 3.0; // Example density in g/cm^3
        double radiusMin = (minDiameterKm * 1e5) / 2.0;
        double radiusMax = (maxDiameterKm * 1e5) / 2.0;
        double volumeMin = (4.0 / 3.0) * M_PI * pow(radiusMin, 3);
        double volumeMax = (4.0 / 3.0) * M_PI * pow(radiusMax, 3);
        double avgVolume = (volumeMin + volumeMax) / 2.0;
        return density * avgVolume; // Approximate mass in grams
    }
};  // Add the closing brace and semicolon to terminate the class definition.

// SFML Hyperbolic Orbit Visualization Functions
const double EARTH_RADIUS = 6371.0;  // Earth radius in kilometers
const double SCALE_FACTOR = 0.01;    // Scaling factor for visualization

double degToRad(double degrees) {
    return degrees * M_PI / 180.0;
}

sf::Vector2f calculatePosition(double semiMajorAxis, double eccentricity, double trueAnomaly, double scaleFactor) {
    double r = (semiMajorAxis * (1 - eccentricity * eccentricity)) / (1 + eccentricity * cos(trueAnomaly));
    double x = r * cos(trueAnomaly);  // X position
    double y = r * sin(trueAnomaly);  // Y position
    return sf::Vector2f(x * scaleFactor, y * scaleFactor);
}

int main() {
    loadEnvFile(".env");

    // Ask the user for a single date
    string selectedDate;
    cout << "\n\nWelcome to the NEO Analyzer!" << endl;
    cout << "Enter a date (YYYY-MM-DD) to search for NEOs: ";
    cin >> selectedDate;

    const char* apiKeyEnv = getenv("API_KEY");  // Get API key from environment variable
    string apiKey = apiKeyEnv ? apiKeyEnv : "";      // If environment variable is missing, use empty string

    if (apiKey.empty()) {
        cerr << "API key is missing. Please set the API_KEY environment variable." << endl;
        return 1;  // Exit program if API key is not set
    }

    json jsonData;
    json selectedNeoJson;

    // Fetch NEO data for the selected date and the API key
    string neo_data = fetch_neo_data(selectedDate, apiKey);

    if (neo_data.empty()) {
        cerr << "Failed to fetch data from NASA API." << endl;
        return 1;
    }

    try {
        jsonData = json::parse(neo_data);  // Parse the JSON response

        // Select the first asteroid for simplicity
        selectedNeoJson = jsonData["near_earth_objects"][selectedDate][0];
        cout << "Selected NEO: " << selectedNeoJson["name"] << endl;

        // Extract relevant data for visualization
        double missDistance = stod(selectedNeoJson["close_approach_data"][0]["miss_distance"]["kilometers"].get<string>());
        double relativeVelocity = stod(selectedNeoJson["close_approach_data"][0]["relative_velocity"]["kilometers_per_second"].get<string>());

        // Visualization parameters for hyperbolic orbit
        double semiMajorAxis = -missDistance;   // Negative for hyperbolic orbits
        double eccentricity = 1.5;              // Arbitrary eccentricity for visualization
        double trueAnomaly = degToRad(-90.0);   // Start at -90 degrees
        double thetaStep = 0.001;               // Angular step for true anomaly

        // SFML window setup
        sf::RenderWindow window(sf::VideoMode(800, 800), "Asteroid Trajectory - Hyperbolic Orbit");
        sf::CircleShape earth(EARTH_RADIUS * SCALE_FACTOR);  // Earth represented as a circle
        earth.setFillColor(sf::Color::Blue);
        earth.setPosition(400 - EARTH_RADIUS * SCALE_FACTOR, 400 - EARTH_RADIUS * SCALE_FACTOR); // Center Earth

        sf::CircleShape asteroid(5.f);  // Asteroid represented as a small circle
        asteroid.setFillColor(sf::Color::White);

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            // Update true anomaly over time
            trueAnomaly += thetaStep;

            // Calculate asteroid's position
            sf::Vector2f position = calculatePosition(semiMajorAxis, eccentricity, trueAnomaly, SCALE_FACTOR);
            asteroid.setPosition(400 + position.x, 400 - position.y);  // Offset to center around Earth

            // Clear and draw
            window.clear();
            window.draw(earth);
            window.draw(asteroid);
            window.display();
        }

    } catch (const std::exception& e) {
        cerr << "Error parsing data: " << e.what() << endl;
        return 1;
    }

    return 0;
}