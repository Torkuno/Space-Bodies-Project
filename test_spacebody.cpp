#include <gtest/gtest.h>
#include "src/planets.h"
#include "src/classes.h"

// Test for the Planet class
TEST(PlanetTest, SurfaceGravity) {
    Planet earth("Earth", 12742.0, 5.972e24); // Diameter in km, mass in kg
    EXPECT_DOUBLE_EQ(earth.calculateSurfaceGravity(), 9.807); // Should match known value
}

TEST(PlanetTest, EscapeVelocity) {
    Planet earth("Earth", 12742.0, 5.972e24);
    EXPECT_DOUBLE_EQ(earth.calculateEscapeVelocity(), 11.186); // Should match known value
}

// Test for the Asteroid class
TEST(AsteroidTest, ConstructorAndMass) {
    json testData = {
        {"name", "TestAsteroid"},
        {"estimated_diameter", {
            {"kilometers", {
                {"estimated_diameter_min", 0.5},
                {"estimated_diameter_max", 1.0}
            }}
        }},
        {"is_potentially_hazardous_asteroid", true},
        {"nasa_jpl_url", "http://example.com"},
        {"absolute_magnitude_h", 25.0},
        {"close_approach_data", {
            {
                {"close_approach_date", "2024-10-10"},
                {"relative_velocity", {{"kilometers_per_second", "5.0"}}},
                {"miss_distance", {{"kilometers", "500000"}}}
            }
        }}
    };

    Asteroid asteroid(testData);
    EXPECT_EQ(asteroid.getDiameter(), 0.5); // Minimum diameter
}

// Test for impact energy calculation
TEST(AsteroidTest, ImpactEnergy) {
    json testData = {
        {"name", "ImpactTest"},
        {"estimated_diameter", {
            {"kilometers", {
                {"estimated_diameter_min", 0.5},
                {"estimated_diameter_max", 1.0}
            }}
        }},
        {"is_potentially_hazardous_asteroid", true},
        {"absolute_magnitude_h", 25.0},
        {"close_approach_data", {
            {
                {"close_approach_date", "2024-10-10"},
                {"relative_velocity", {{"kilometers_per_second", "5.0"}}},
                {"miss_distance", {{"kilometers", "500000"}}}
            }
        }}
    };

    Asteroid asteroid(testData);
    EXPECT_GT(asteroid.calculateImpactEnergy(), 0); // Ensure energy is positive
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
