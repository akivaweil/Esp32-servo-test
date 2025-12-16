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
    // XSHUT is active LOW - LOW = sensor disabled, HIGH = sensor enabled
    pinMode(TOF_XSHUT_PIN, OUTPUT);
    digitalWrite(TOF_XSHUT_PIN, LOW);  // Pull LOW to reset/disable sensor
    delay(50); // Longer delay for reset
    digitalWrite(TOF_XSHUT_PIN, HIGH); // Pull HIGH to enable sensor
    delay(100); // Give sensor time to boot up after enable
    
    // Initialize I2C communication with custom pins
    // Set slower clock speed for better reliability (100kHz)
    Wire.begin(TOF_SDA_PIN, TOF_SCL_PIN);
    Wire.setClock(100000); // 100kHz I2C speed (slower = more reliable)
    delay(200); // Give I2C more time to stabilize
    
    // Full I2C bus scan to find all devices
    Serial.println("Scanning I2C bus...");
    byte devicesFound = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
            devicesFound++;
        }
    }
    if (devicesFound == 0) {
        Serial.println("No I2C devices found! Check wiring.");
    }
    
    // Check specifically for VL53L0X at 0x29
    Serial.print("Checking VL53L0X at 0x29... ");
    Wire.beginTransmission(0x29);
    byte error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("Found!");
    } else {
        Serial.print("Not found (error: ");
        Serial.print(error);
        Serial.println(")");
        Serial.println("Skipping ToF initialization - device not responding");
        tofInitialized = false;
        return;
    }
    
    // Additional delay to ensure sensor is ready
    delay(50);
    
    // Initialize the VL53L0X sensor with timeout protection
    Serial.print("Initializing VL53L0X... ");
    unsigned long initStart = millis();
    bool initResult = tofSensor.init();
    unsigned long initDuration = millis() - initStart;
    
    if (!initResult) {
        Serial.println("FAILED!");
        Serial.print("Init took ");
        Serial.print(initDuration);
        Serial.println("ms");
        Serial.println("Check wiring: SDA->GPIO10, SCL->GPIO11, VIN->3.3V, GND->GND");
        tofInitialized = false;
        return;
    }
    
    Serial.print("OK (");
    Serial.print(initDuration);
    Serial.println("ms)");
    
    // Set measurement timeout (in milliseconds)
    // Longer timeout = longer range but slower readings
    tofSensor.setTimeout(500);
    
    // Use single-shot mode instead of continuous (more reliable)
    // Continuous mode can sometimes have issues
    // Try longer timing budget for better accuracy
    tofSensor.setMeasurementTimingBudget(33000); // 33ms timing budget (default)
    
    tofInitialized = true;
    Serial.println("ToF sensor initialized successfully!");
    Serial.println("Place an object 5-200cm in front of sensor and type 'ToF' to test");
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
    // But also check for minimum range (sensor needs at least 30mm)
    if (distanceMm < 30 || distanceMm > 2000) {
        return 0.0; // Out of range
    }
    
    return (float)distanceMm;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔍 READ RAW DISTANCE (FOR DEBUGGING)                                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

float readToFRawDistance() {
    if (!tofInitialized) {
        return 0.0;
    }
    
    // Read raw value without filtering for debugging
    uint16_t distanceMm = tofSensor.readRangeSingleMillimeters();
    bool timeout = tofSensor.timeoutOccurred();
    
    return (float)distanceMm; // Return raw value even if out of range
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📏 READ DISTANCE (CENTIMETERS)                                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

float readToFDistanceCm() {
    float distanceMm = readToFDistance();
    return distanceMm / 10.0; // Convert mm to cm
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📏 READ DISTANCE (INCHES)                                             ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

float readToFDistanceInches() {
    float distanceMm = readToFDistance();
    return distanceMm / 25.4; // Convert mm to inches
}

