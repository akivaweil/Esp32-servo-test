#include "Ultrasonic_Sensor.h"
#include "Config/Pins_Definitions.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 ULTRASONIC SENSOR IMPLEMENTATION                                   ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// How Ultrasonic Sensors Work:
// 1. Send a 10μs pulse on TRIG pin
// 2. Sensor sends out ultrasonic sound waves
// 3. Sound bounces off object and returns
// 4. ECHO pin goes HIGH for duration = round trip time
// 5. Distance = (time × speed of sound) / 2
//    Speed of sound ≈ 343 m/s = 0.0343 cm/μs
//    So: distance (cm) = (time_us / 2) / 29.1

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔧 INITIALIZATION                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void initializeUltrasonic() {
    // Set TRIG pin as OUTPUT (sends the trigger pulse)
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    // Set ECHO pin as INPUT (receives the echo signal)
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    
    // Initialize TRIG pin to LOW
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2); // Small delay for stability
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📏 READ DISTANCE (CENTIMETERS)                                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

float readDistance() {
    // Step 1: Clear the TRIG pin
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    
    // Step 2: Send a 10μs HIGH pulse on TRIG pin to trigger the sensor
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10); // 10μs trigger pulse
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    
    // Step 3: Read the ECHO pin - pulseIn() measures how long ECHO stays HIGH
    // This duration = round trip time for the sound wave
    // timeout: 30000μs = 30ms (max range ~5 meters)
    unsigned long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);
    
    // Step 4: Calculate distance
    // duration is in microseconds
    // Speed of sound = 343 m/s = 0.0343 cm/μs
    // Distance = (duration / 2) / 29.1 cm
    // We divide by 2 because duration is round trip time
    float distance = (duration / 2.0) / 29.1;
    
    // If no echo received (timeout), return 0 or max range
    if (duration == 0) {
        return 0.0; // No object detected or out of range
    }
    
    return distance;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📏 READ DISTANCE (INCHES)                                             ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

float readDistanceInches() {
    float distanceCm = readDistance();
    return distanceCm / 2.54; // Convert cm to inches
}

