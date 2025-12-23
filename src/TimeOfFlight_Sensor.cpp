#include "TimeOfFlight_Sensor.h"
#include "config/Config.h"
#include <Wire.h>
#include <VL53L0X.h>

// Forward declaration for OTA update
void updateOTA();

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 TIME OF FLIGHT SENSOR IMPLEMENTATION                                ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

VL53L0X tofSensor;
bool tofInitialized = false;

// Simplified initialization state machine
enum ToFInitState {
    TOF_INIT_NOT_STARTED,
    TOF_INIT_RESETTING,
    TOF_INIT_INITIALIZING,
    TOF_INIT_COMPLETE,
    TOF_INIT_FAILED
};

ToFInitState tofInitState = TOF_INIT_NOT_STARTED;
unsigned long tofInitStartTime = 0;
bool i2cSetup = false;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔧 INITIALIZATION (NON-BLOCKING STATE MACHINE)                       ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void updateToFInit() {
    switch (tofInitState) {
        case TOF_INIT_NOT_STARTED:
            Serial.println("Initializing ToF sensor...");
            pinMode(TOF_XSHUT_PIN, OUTPUT);
            digitalWrite(TOF_XSHUT_PIN, LOW);
            tofInitStartTime = millis();
            tofInitState = TOF_INIT_RESETTING;
            break;
            
        case TOF_INIT_RESETTING:
            // Reset delay
            if (millis() - tofInitStartTime >= 50) {
                digitalWrite(TOF_XSHUT_PIN, HIGH);
                tofInitStartTime = millis();
                i2cSetup = false;
                tofInitState = TOF_INIT_INITIALIZING;
            }
            break;
            
        case TOF_INIT_INITIALIZING:
            // Wait for enable delay, then setup I2C
            if (millis() - tofInitStartTime >= 100 && !i2cSetup) {
                Wire.begin(TOF_SDA_PIN, TOF_SCL_PIN);
                Wire.setClock(100000);
                i2cSetup = true;
            }
            
            // Wait for I2C to stabilize
            if (millis() - tofInitStartTime < 300) {
                break;
            }
            
            // I2C ready, proceed with OTA window and init
            
            // Service OTA before blocking init call
            updateOTA();
            
            // Check if we should proceed with init (allow OTA window)
            if (millis() - tofInitStartTime < 10300) {
                // Still in OTA window, keep servicing
                break;
            }
            
            // Now do the blocking init
            {
                updateOTA();
                Serial.print("Initializing VL53L0X... ");
                unsigned long initStart = millis();
                bool initResult = tofSensor.init();
                updateOTA();
                unsigned long initDuration = millis() - initStart;
                
                if (initDuration > TOF_INIT_TIMEOUT_MS || !initResult) {
                    Serial.println("FAILED!");
                    Serial.println("Check wiring: SDA->GPIO10, SCL->GPIO11, VIN->3.3V, GND->GND");
                    tofInitState = TOF_INIT_FAILED;
                    tofInitialized = false;
                } else {
                    Serial.print("OK (");
                    Serial.print(initDuration);
                    Serial.println("ms)");
                    tofSensor.setTimeout(500);
                    tofSensor.setMeasurementTimingBudget(33000);
                    tofInitialized = true;
                    tofInitState = TOF_INIT_COMPLETE;
                    Serial.println("ToF sensor ready!");
                }
            }
            break;
            
        case TOF_INIT_COMPLETE:
        case TOF_INIT_FAILED:
            // Done
            break;
    }
}

void initializeToF() {
    if (tofInitState == TOF_INIT_NOT_STARTED) {
        // Will start on first call to updateToFInit()
    }
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

