// src/planets.h

#ifndef PLANETS_H
#define PLANETS_H

#include <vector>
#include <string>

using namespace std;

// Struct to hold planet data
struct PlanetData {
    string name;
    double diameter; // in kilometers
    double mass;     // in kilograms
};

namespace SolarSystem {
    // Predefined data for planets
    const vector<PlanetData> predefinedPlanets = {
        {"Mercury", 4879.4, 3.3011e23},
        {"Venus", 12104, 4.8675e24},
        {"Earth", 12742, 5.97237e24},
        {"Mars", 6779, 6.4171e23},
        {"Jupiter", 139820, 1.8982e27},
        {"Saturn", 116460, 5.6834e26},
        {"Uranus", 50724, 8.6810e25},
        {"Neptune", 49244, 1.02413e26},
    };
}

#endif 
