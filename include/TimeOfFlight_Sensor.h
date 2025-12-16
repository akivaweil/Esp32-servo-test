#ifndef TIME_OF_FLIGHT_SENSOR_H
#define TIME_OF_FLIGHT_SENSOR_H

#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 TIME OF FLIGHT SENSOR CONFIGURATION                                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Function Declarations
void initializeToF();
float readToFDistance(); // Returns distance in millimeters
float readToFDistanceCm(); // Returns distance in centimeters
float readToFDistanceInches(); // Returns distance in inches
float readToFRawDistance(); // Returns raw distance for debugging (no filtering)
bool isToFReady(); // Check if sensor is ready

#endif





