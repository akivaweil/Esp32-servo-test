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
    // Initialize I2C communication with custom pins
    Wire.begin(TOF_SDA_PIN, TOF_SCL_PIN);
    delay(100); // Give I2C time to stabilize
    
    // Scan I2C bus to check if sensor is present
    Serial.print("Scanning I2C bus... ");
    Wire.beginTransmission(0x29); // VL53L0X default I2C address
    byte error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("Sensor found at address 0x29");
    } else {
        Serial.print("No device found at 0x29 (error: ");
        Serial.print(error);
        Serial.println(")");
    }
    
    // Initialize the VL53L0X sensor
    if (!tofSensor.init()) {
        Serial.println("Failed to initialize ToF sensor!");
        Serial.println("Check wiring: SDA->GPIO10, SCL->GPIO11, VIN->3.3V, GND->GND");
        tofInitialized = false;
        return;
    }
    
    // Set measurement timeout (in milliseconds)
    // Longer timeout = longer range but slower readings
    tofSensor.setTimeout(500);
    
    // Use single-shot mode instead of continuous (more reliable)
    // Continuous mode can sometimes have issues
    tofSensor.setMeasurementTimingBudget(20000); // 20ms timing budget for faster readings
    
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
    
    // Read distance in millimeters using single-shot mode
    // Single-shot is more reliable than continuous mode
    uint16_t distanceMm = tofSensor.readRangeSingleMillimeters();
    
    // Check for timeout (sensor couldn't detect object)
    if (tofSensor.timeoutOccurred()) {
        return 0.0; // No object detected or out of range
    }
    
    // VL53L0X can return very large values when out of range
    // Valid range is typically 30mm to 2000mm
    if (distanceMm > 2000) {
        return 0.0; // Out of range
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
