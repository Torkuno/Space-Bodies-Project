#ifndef CLASS_H
#define CLASS_H

#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <vector>
#include "platform_config.h"  // For conditional compilation

using namespace std;

// Base class for Space Bodies (e.g., Planets, Asteroids)
class SpaceBody {
public:
    SpaceBody(const string& name, double diameter, double mass);
    virtual void printInfo() const;
    double calculateSurfaceGravity() const;
    double calculateEscapeVelocity() const;
    double getMass() const;
    double getDiameter() const;
    virtual ~SpaceBody();

protected:
    string name;
    double diameter;  // in kilometers
    double mass;      // in kilograms
};

// Derived class for Planets
class Planet : public SpaceBody {
public:
    Planet(const string& name, double diameter, double mass);
    void printInfo() const override;
    ~Planet();
};

// Derived class for Asteroids
class Asteroid : public SpaceBody {
public:
    Asteroid(const json& asteroidData);
    Asteroid(const Asteroid& other);
    void printInfo() const override;
    double calculateImpactEnergy() const;
    Asteroid operator+(const Asteroid& other) const;
    ~Asteroid();

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

    static double calculateMass(const json& asteroidData);
};

#endif
