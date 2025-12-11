#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 ULTRASONIC SENSOR CONFIGURATION                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Function Declarations
void initializeUltrasonic();
float readDistance(); // Returns distance in centimeters
float readDistanceInches(); // Returns distance in inches

#endif



