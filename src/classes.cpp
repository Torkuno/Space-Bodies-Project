// src/class.cpp

#include "classes.h"

// SpaceBody constructor
SpaceBody::SpaceBody(const string& name, double diameter, double mass)
    : name(name), diameter(diameter), mass(mass) {}

// SpaceBody printInfo method
void SpaceBody::printInfo() const {
    cout << "Name: " << name << ", Diameter: " << diameter << " km, Mass: " << mass << " kg" << endl;
}

// Calculate surface gravity
double SpaceBody::calculateSurfaceGravity() const {
    const double G = 6.67430e-11;  // Gravitational constant (m^3 kg^-1 s^-2)
    double radius_m = (diameter * 1000) / 2.0;  // Convert km to meters and get radius
    return (G * mass) / (radius_m * radius_m);  // Surface gravity in m/s^2
}

// Calculate escape velocity
double SpaceBody::calculateEscapeVelocity() const {
    const double G = 6.67430e-11;  // Gravitational constant (m^3 kg^-1 s^-2)
    double radiusMeters = (diameter * 1000) / 2.0;  // Convert km to meters and get radius
    double escapeVelocity_m_s = sqrt((2 * G * mass) / radiusMeters);  // Escape velocity in m/s
    return escapeVelocity_m_s / 1000.0;  // Convert to km/s
}

// Getter for mass
double SpaceBody::getMass() const { return mass; }

// Getter for diameter
double SpaceBody::getDiameter() const { return diameter; }

// SpaceBody destructor
SpaceBody::~SpaceBody() {
    cout << "Space body " << name << " memory freed." << endl;
}

// Planet constructor
Planet::Planet(const string& name, double diameter, double mass)
    : SpaceBody(name, diameter, mass) {}

// Planet printInfo method
void Planet::printInfo() const {
    cout << "Planet Name: " << name << ", Mass: " << mass << " kg, Diameter: " << diameter << " km" << endl;
    cout << "Surface Gravity: " << calculateSurfaceGravity() << " m/s^2" << endl;
    cout << "Escape Velocity: " << calculateEscapeVelocity() << " km/s" << endl;
}

// Planet destructor
Planet::~Planet() {
    cout << "Planet " << name << " memory freed." << endl;
}

// Asteroid constructor (from json)
Asteroid::Asteroid(const json& asteroidData)
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

// Asteroid copy constructor
Asteroid::Asteroid(const Asteroid& other)
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

// Asteroid printInfo method
void Asteroid::printInfo() const {
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
}

// Calculate impact energy in megatons of TNT
double Asteroid::calculateImpactEnergy() const {
    double velocity_m_s = relativeVelocityKmPerS * 1000.0;  // Convert km/s to m/s
    double energy_joules = 0.5 * mass * pow(velocity_m_s, 2);  // Energy in Joules
    double energy_megatons = energy_joules / 4.184e15; // Convert Joules to megatons of TNT
    return energy_megatons;
}

// Operator overload for adding two Asteroids
Asteroid Asteroid::operator+(const Asteroid& other) const {
    Asteroid combinedAsteroid(*this); // Create a copy of the current asteroid
    combinedAsteroid.name = name + " & " + other.name; // Combine names
    combinedAsteroid.minDiameterKm += other.minDiameterKm;
    combinedAsteroid.maxDiameterKm += other.maxDiameterKm;
    combinedAsteroid.mass += other.mass;
    combinedAsteroid.relativeVelocityKmPerS += other.relativeVelocityKmPerS;
    combinedAsteroid.missDistanceKm += other.missDistanceKm;
    combinedAsteroid.isDangerous = ((combinedAsteroid.minDiameterKm > 280) || (combinedAsteroid.relativeVelocityKmPerS > 5.0));
    return combinedAsteroid;
}

// Asteroid destructor
Asteroid::~Asteroid() {
    cout << "Asteroid " << name << " memory freed." << endl;
}

// Static helper function to calculate mass
double Asteroid::calculateMass(const json& asteroidData) {
    const double density = 3000.0; // Example density in kg/m^3
    double diameterMin_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_min"].get<double>() * 1000.0; // km to m
    double diameterMax_m = asteroidData["estimated_diameter"]["kilometers"]["estimated_diameter_max"].get<double>() * 1000.0; // km to m
    double volumeMin = (4.0 / 3.0) * M_PI * pow(diameterMin_m / 2.0, 3);
    double volumeMax = (4.0 / 3.0) * M_PI * pow(diameterMax_m / 2.0, 3);
    double massMin = density * volumeMin; // kg
    double massMax = density * volumeMax; // kg
    return (massMin + massMax) / 2.0; // Return average mass
}
