#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "src/get_data.h"
#include "json.hpp"

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
    SpaceBody(const std::string& name, double diameter) : name(name), diameter(diameter) {}

    virtual void printInfo() {
        std::cout << "Name: " << name << ", diameter: " << diameter << std::endl;
    }

    virtual ~SpaceBody() {
        std::cout << "Space body " << name << " destroyed." << std::endl;
    }

protected:
    std::string name;
    double diameter;
};

// Asteroid Class Inheriting from SpaceBody
class Asteroid : public SpaceBody {
public:
    Asteroid(const nlohmann::json& asteroidData) : SpaceBody(asteroidData["name"], asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"]),
        id(asteroidData["id"]),
        nasa_jpl_url(asteroidData["nasa_jpl_url"]),
        absolute_magnitude(asteroidData["absolute_magnitude_h"]),
        isDangerous(asteroidData["is_potentially_hazardous_asteroid"]) {

        auto diameter = asteroidData["estimated_diameter"];
        minDiameterKm = diameter["kilometers"]["estimated_diameter_min"];
        maxDiameterKm = diameter["kilometers"]["estimated_diameter_max"];

        auto close_approach = asteroidData["close_approach_data"][0];
        closeApproachDate = close_approach["close_approach_date"];
        relativeVelocityKmPerS = std::stod(close_approach["relative_velocity"]["kilometers_per_second"].get<std::string>());
        missDistanceKm = std::stod(close_approach["miss_distance"]["kilometers"].get<std::string>());

        // Adjusted miss distance for visualization
        missDistanceKm = std::max(missDistanceKm / 2.0, EARTH_RADIUS * 2);  // Ensure it starts outside Earth's radius

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

    double calculateMass() const {
        const double density = 3.0; // Example density in g/cm^3
        double radiusMin = (minDiameterKm * 1e5) / 2.0;
        double radiusMax = (maxDiameterKm * 1e5) / 2.0;
        double volumeMin = (4.0 / 3.0) * M_PI * std::pow(radiusMin, 3);
        double volumeMax = (4.0 / 3.0) * M_PI * std::pow(radiusMax, 3);
        double avgVolume = (volumeMin + volumeMax) / 2.0;
        return density * avgVolume;
    }

    double getMissDistanceKm() const {
        return missDistanceKm;
    }

private:
    std::string id;
    std::string nasa_jpl_url;
    double absolute_magnitude;
    double minDiameterKm;
    double maxDiameterKm;
    bool isDangerous;
    std::string closeApproachDate;
    double relativeVelocityKmPerS;
    double missDistanceKm;
    double mass;
};

// Convert degrees to radians
double degToRad(double degrees) {
    return degrees * M_PI / 180.0;
}

// Function to calculate asteroid position based on semi-major axis and true anomaly
sf::Vector2f calculatePosition(double semiMajorAxis, double eccentricity, double trueAnomaly, double scaleFactor) {
    double r = (semiMajorAxis * (1 - eccentricity * eccentricity)) / (1 + eccentricity * std::cos(trueAnomaly));
    double x = r * std::cos(trueAnomaly);  // X position
    double y = r * std::sin(trueAnomaly);  // Y position

    return sf::Vector2f(x * scaleFactor, y * scaleFactor);
}

// Function to handle slider movement
void handleSlider(sf::RectangleShape& sliderBar, sf::CircleShape& slider, sf::RenderWindow& window, float& timeElapsed) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && draggingSlider) {
        // Limit the slider's movement within the slider bar
        float newPosX = static_cast<float>(mousePos.x);
        if (newPosX < sliderBar.getPosition().x)
            newPosX = sliderBar.getPosition().x;
        if (newPosX > sliderBar.getPosition().x + sliderBar.getSize().x)
            newPosX = sliderBar.getPosition().x + sliderBar.getSize().x;

        slider.setPosition(newPosX, slider.getPosition().y);
        // Update timeElapsed based on slider position
        timeElapsed = (newPosX - sliderBar.getPosition().x) / sliderBar.getSize().x * 360.0f;  // Scale to [0, 360] degrees for orbit
    }
}

int main() {
    loadEnvFile(".env");

    // Ask for the date to search NEOs
    std::string selectedDate;
    std::cout << "Enter a date (YYYY-MM-DD) to search for NEOs: ";
    std::cin >> selectedDate;

    const char* apiKeyEnv = getenv("API_KEY");  // Get API key from environment variable
    std::string apiKey = apiKeyEnv ? apiKeyEnv : "";  // If environment variable is missing, use an empty string

    if (apiKey.empty()) {
        std::cerr << "API key is missing. Please set the API_KEY environment variable." << std::endl;
        return 1;  // Exit if the API key is not set
    }

    nlohmann::json jsonData, selectedNeoJson;
    std::string neo_data = fetch_neo_data(selectedDate, apiKey);  // Fetch NEO data from the API

    if (neo_data.empty()) {
        std::cerr << "Failed to fetch data from NASA API." << std::endl;
        return 1;
    }

    try {
        jsonData = nlohmann::json::parse(neo_data);  // Parse the JSON response
        auto& neos = jsonData["near_earth_objects"][selectedDate];
        std::cout << "There are " << neos.size() << " NEOs for the date " << selectedDate << ".\n";

        for (size_t i = 0; i < neos.size(); ++i) {
            std::cout << i + 1 << ". " << neos[i]["name"] << std::endl;
        }

        int choice;
        std::cout << "\nSelect a NEO by number: ";
        std::cin >> choice;

        if (choice < 1 || choice > neos.size()) {
            std::cerr << "Invalid selection." << std::endl;
            return 1;
        }

        selectedNeoJson = neos[choice - 1];

        Asteroid asteroid(selectedNeoJson);
        asteroid.printInfo();

        // Visualization parameters for elliptical orbit
        double semiMajorAxis = asteroid.getMissDistanceKm();  // Positive semi-major axis for elliptical orbit
        double eccentricity = 0.5;  // Eccentricity for a smoother elliptical orbit
        double trueAnomaly = degToRad(-90.0);  // Start at -90 degrees

        // Vector to store the asteroid's path for drawing the orbit
        std::vector<sf::Vector2f> orbitPath;

        // Setup SFML window
        sf::RenderWindow window(sf::VideoMode(800, 800), "Asteroid Elliptical Orbit Visualization");

        // Load Earth texture
        sf::Texture earthTexture;
        if (!earthTexture.loadFromFile("Earth_Image.jpeg")) {
            std::cerr << "Failed to load Earth image." << std::endl;
            return 1;
        }

        sf::CircleShape earth(100.f);  // Earth with realistic size
        earth.setTexture(&earthTexture);  // Set Earth image as texture
        earth.setPosition(WINDOW_CENTER_X - 100.f, WINDOW_CENTER_Y - 100.f); // Center Earth

        // Asteroid size increased for visibility
        sf::CircleShape asteroidShape(10.f);
        asteroidShape.setFillColor(sf::Color::White);

        // Create a slider bar
        sf::RectangleShape sliderBar(sf::Vector2f(200, 5));
        sliderBar.setFillColor(sf::Color::White);
        sliderBar.setPosition(300, 750);

        // Create a slider (circle) for time control
        sf::CircleShape slider(10.f);
        slider.setFillColor(sf::Color::Red);
        slider.setPosition(sliderBar.getPosition().x, sliderBar.getPosition().y - 5);  // Position on top of the slider bar

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();

                // Start dragging the slider if the user clicks on it
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (slider.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                        draggingSlider = true;
                    }
                }

                // Stop dragging when the mouse is released
                if (event.type == sf::Event::MouseButtonReleased) {
                    draggingSlider = false;
                }
            }

            // Handle slider movement and update timeElapsed
            handleSlider(sliderBar, slider, window, timeElapsed);

            // Calculate the true anomaly based on the slider's value (mapped to 0-360 degrees)
            trueAnomaly = degToRad(timeElapsed);
            std::cout << "True Anomaly: " << trueAnomaly << std::endl;

            // Calculate asteroid position and store it in orbit path
            sf::Vector2f position = calculatePosition(semiMajorAxis, eccentricity, trueAnomaly, SCALE_FACTOR);
            std::cout << "Asteroid Position: (" << position.x << ", " << position.y << ")" << std::endl;
            asteroidShape.setPosition(WINDOW_CENTER_X + position.x, WINDOW_CENTER_Y - position.y);

            // Store position dynamically as the asteroid moves
            orbitPath.push_back(asteroidShape.getPosition());

            window.clear();
            window.draw(earth);
            window.draw(asteroidShape);

            // Draw the orbit path as a series of connected lines
            sf::VertexArray orbitLines(sf::LineStrip, orbitPath.size());
            for (size_t i = 0; i < orbitPath.size(); ++i) {
                orbitLines[i].position = orbitPath[i];
                orbitLines[i].color = sf::Color::White;
            }
            window.draw(orbitLines);

            // Draw the slider bar and slider (time control)
            window.draw(sliderBar);
            window.draw(slider);

            window.display();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing data: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}