// main_test.cpp
#include <gtest/gtest.h>
#include "main.cpp"  // Include the skeleton functions from main.cpp

// Test perihelion and aphelion calculation
TEST(OrbitCalculations, PerihelionAphelion) {
    double a = 2.0;  // semi-major axis in AU
    double e = 0.5;  // eccentricity
    double expected_perihelion = a * (1 - e);
    double expected_aphelion = a * (1 + e);

    ASSERT_DOUBLE_EQ(OrbitCalculations::PerihelionAphelion::calculate_perihelion(a, e), expected_perihelion);
    ASSERT_DOUBLE_EQ(OrbitCalculations::PerihelionAphelion::calculate_aphelion(a, e), expected_aphelion);
}

// Edge case tests for perihelion/aphelion
TEST(OrbitCalculations, PerihelionAphelionEdgeCases) {
    ASSERT_THROW(OrbitCalculations::PerihelionAphelion::calculate_perihelion(-1.0, 0.5), std::invalid_argument);  // Invalid semi-major axis
    ASSERT_THROW(OrbitCalculations::PerihelionAphelion::calculate_aphelion(2.0, 1.5), std::invalid_argument);   // Invalid eccentricity (e > 1)
}

// Test minimum distance calculation
TEST(CloseApproachAnalysis, MinimumDistance) {
    double neo_distance = CloseApproachAnalysis::calculate_minimum_distance_to_earth(0.002);  // example params
    double expected_distance = 0.002;  // in AU
    ASSERT_NEAR(neo_distance, expected_distance, 1e-6);
}

// Edge case for minimum distance calculation
TEST(CloseApproachAnalysis, MinimumDistanceEdgeCases) {
    ASSERT_THROW(CloseApproachAnalysis::calculate_minimum_distance_to_earth(-0.1), std::invalid_argument);  // Negative distance
}

// Test kinetic energy calculation
TEST(ImpactRiskAssessment, KineticEnergy) {
    double mass = 1e12;  // example mass in kg
    double velocity = 20000;  // velocity in m/s
    double expected_energy = 0.5 * mass * velocity * velocity;

    ASSERT_NEAR(ImpactRiskAssessment::calculate_kinetic_energy(mass, velocity), expected_energy, 1e9);
}

// Edge case for kinetic energy calculation
TEST(ImpactRiskAssessment, KineticEnergyEdgeCases) {
    ASSERT_THROW(ImpactRiskAssessment::calculate_kinetic_energy(-1e12, 20000), std::invalid_argument);  // Negative mass
    ASSERT_THROW(ImpactRiskAssessment::calculate_kinetic_energy(1e12, -20000), std::invalid_argument);  // Negative velocity
}

// Test heliocentric velocity at perihelion
TEST(VelocityCalculations, HeliocentricVelocity) {
    double semi_major_axis = 1.5;  // in AU
    double velocity_at_perihelion = OrbitCalculations::Velocity::calculate_velocity_at_perihelion(semi_major_axis);

    ASSERT_GT(velocity_at_perihelion, 0.0);  // Check that the velocity is positive
}

// Edge case for velocity calculation
TEST(VelocityCalculations, HeliocentricVelocityEdgeCases) {
    ASSERT_THROW(OrbitCalculations::Velocity::calculate_velocity_at_perihelion(-1.5), std::invalid_argument);  // Invalid semi-major axis
}

// Test orbit intersection with Earth's orbit
TEST(OrbitIntersection, IntersectsEarthOrbit) {
    bool does_intersect = OrbitIntersection::check_orbit_intersection(0.1, 1.0);  // example params
    ASSERT_TRUE(does_intersect);  // Assuming the NEO intersects Earth's orbit
}

// Edge case for orbit intersection
TEST(OrbitIntersection, IntersectsEarthOrbitEdgeCases) {
    ASSERT_THROW(OrbitIntersection::check_orbit_intersection(-1.0, 1.0), std::invalid_argument);  // Invalid parameters
}

// Test surface gravity calculation
TEST(SurfaceGravity, CalculateSurfaceGravity) {
    double mass = 1e12;  // in kg
    double radius = 500;  // in meters
    double g = SurfaceGravity::calculate_surface_gravity(mass, radius);

    double expected_gravity = (G * mass) / (radius * radius);  // Use gravitational constant G
    ASSERT_NEAR(g, expected_gravity, 1e-6);
}

// Edge case for surface gravity calculation
TEST(SurfaceGravity, CalculateSurfaceGravityEdgeCases) {
    ASSERT_THROW(SurfaceGravity::calculate_surface_gravity(-1e12, 500), std::invalid_argument);  // Negative mass
    ASSERT_THROW(SurfaceGravity::calculate_surface_gravity(1e12, -500), std::invalid_argument);  // Negative radius
    ASSERT_THROW(SurfaceGravity::calculate_surface_gravity(1e12, 0), std::invalid_argument);     // Zero radius (division by zero)
}

// Main function to run all the tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
