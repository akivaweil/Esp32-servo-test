#include "TimeOfFlight_Sensor.h"
#include "config/Config.h"
#include <Wire.h>
#include <VL53L0X.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 TIME OF FLIGHT SENSOR IMPLEMENTATION                                ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// How Time of Flight (ToF) Sensors Work:
// 1. Sensor emits infrared laser pulses
// 2. Light reflects off target object
// 3. Sensor measures time for light to return
// 4. Distance = (speed of light × time) / 2
// 5. VL53L0X uses I2C communication (SDA/SCL pins)
// 6. Range: ~30mm to 2000mm (2 meters)
// 7. More accurate than ultrasonic, works in sunlight

VL53L0X tofSensor;
bool tofInitialized = false;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔧 INITIALIZATION                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void initializeToF() {
    // Configure XSHUT pin (shutdown control)
    // HIGH = sensor enabled, LOW = sensor disabled/reset
    pinMode(TOF_XSHUT_PIN, OUTPUT);
    digitalWrite(TOF_XSHUT_PIN, LOW);  // Pull LOW to reset sensor
    delay(10);
    digitalWrite(TOF_XSHUT_PIN, HIGH); // Pull HIGH to enable sensor
    delay(10);
    
    // Initialize I2C communication with custom pins
    Wire.begin(TOF_SDA_PIN, TOF_SCL_PIN);
    
    // Initialize the VL53L0X sensor
    if (!tofSensor.init()) {
        Serial.println("Failed to initialize ToF sensor!");
        tofInitialized = false;
        return;
    }
    
    // Set measurement timeout (in milliseconds)
    // Longer timeout = longer range but slower readings
    tofSensor.setTimeout(500);
    
    // Start continuous measurement mode
    // This allows faster readings
    tofSensor.startContinuous();
    
    tofInitialized = true;
    Serial.println("ToF sensor initialized successfully!");
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ✅ CHECK IF SENSOR IS READY                                            ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

bool isToFReady() {
    return tofInitialized;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📏 READ DISTANCE (MILLIMETERS)                                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

float readToFDistance() {
    if (!tofInitialized) {
        return 0.0;
    }
    
    // Read distance in millimeters
    // readRangeContinuousMillimeters() is faster than readRangeSingleMillimeters()
    uint16_t distanceMm = tofSensor.readRangeContinuousMillimeters();
    
    // Check for timeout (sensor couldn't detect object)
    if (tofSensor.timeoutOccurred()) {
        return 0.0; // No object detected or out of range
    }
    
    return (float)distanceMm;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📏 READ DISTANCE (CENTIMETERS)                                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

float readToFDistanceCm() {
    float distanceMm = readToFDistance();
    return distanceMm / 10.0; // Convert mm to cm
}
