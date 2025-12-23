#include "TimeOfFlight_Sensor.h"
#include "config/Config.h"
#include <Wire.h>
#include <VL53L0X.h>

// Forward declaration for OTA update
void updateOTA();

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

// ToF initialization state machine
enum ToFInitState {
    TOF_INIT_NOT_STARTED,
    TOF_INIT_RESET_DELAY,
    TOF_INIT_ENABLE_DELAY,
    TOF_INIT_I2C_DELAY,
    TOF_INIT_SCANNING,
    TOF_INIT_CHECKING,
    TOF_INIT_READY_DELAY,
    TOF_INIT_CALLING_INIT,
    TOF_INIT_CONFIGURING,
    TOF_INIT_COMPLETE,
    TOF_INIT_FAILED
};

ToFInitState tofInitState = TOF_INIT_NOT_STARTED;
unsigned long tofInitStartTime = 0;
byte tofScanAddress = 1;
bool tofDeviceFound = false;
bool tofAddressPrinted = false;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔧 INITIALIZATION (NON-BLOCKING STATE MACHINE)                       ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Non-blocking initialization that can be called from loop()
void updateToFInit() {
    switch (tofInitState) {
        case TOF_INIT_NOT_STARTED:
            Serial.println("Initializing ToF sensor (OTA available during init)...");
            pinMode(TOF_XSHUT_PIN, OUTPUT);
            digitalWrite(TOF_XSHUT_PIN, LOW);
            tofInitStartTime = millis();
            tofInitState = TOF_INIT_RESET_DELAY;
            break;
            
        case TOF_INIT_RESET_DELAY:
            if (millis() - tofInitStartTime >= 50) {
                digitalWrite(TOF_XSHUT_PIN, HIGH);
                tofInitStartTime = millis();
                tofInitState = TOF_INIT_ENABLE_DELAY;
            }
            break;
            
        case TOF_INIT_ENABLE_DELAY:
            if (millis() - tofInitStartTime >= 100) {
                Wire.begin(TOF_SDA_PIN, TOF_SCL_PIN);
                Wire.setClock(100000);
                tofInitStartTime = millis();
                tofInitState = TOF_INIT_I2C_DELAY;
            }
            break;
            
        case TOF_INIT_I2C_DELAY:
            if (millis() - tofInitStartTime >= 200) {
                Serial.println("Scanning I2C bus...");
                tofScanAddress = 1;
                tofDeviceFound = false;
                tofAddressPrinted = false;
                tofInitState = TOF_INIT_SCANNING;
            }
            break;
            
        case TOF_INIT_SCANNING:
            // Scan I2C bus incrementally (check a few addresses per loop)
            for (byte i = 0; i < 10 && tofScanAddress < 127; i++, tofScanAddress++) {
                Wire.beginTransmission(tofScanAddress);
                byte error = Wire.endTransmission();
                if (error == 0) {
                    tofDeviceFound = true;
                    if (tofScanAddress == 0x29 && !tofAddressPrinted) {
                        // Only print when we find our target device (once)
                        Serial.print("I2C device found at address 0x");
                        if (tofScanAddress < 16) Serial.print("0");
                        Serial.println(tofScanAddress, HEX);
                        tofAddressPrinted = true;
                        break; // Found our sensor
                    }
                }
            }
            
            if (tofScanAddress >= 127) {
                if (!tofDeviceFound) {
                    Serial.println("No I2C devices found! Check wiring.");
                }
                Serial.print("Checking VL53L0X at 0x29... ");
                tofInitState = TOF_INIT_CHECKING;
            }
            break;
            
        case TOF_INIT_CHECKING: {
            Wire.beginTransmission(0x29);
            byte error = Wire.endTransmission();
            if (error == 0) {
                Serial.println("Found!");
                tofInitStartTime = millis();
                tofInitState = TOF_INIT_READY_DELAY;
            } else {
                Serial.print("Not found (error: ");
                Serial.print(error);
                Serial.println(")");
                Serial.println("Skipping ToF initialization - device not responding");
                tofInitState = TOF_INIT_FAILED;
                tofInitialized = false;
            }
            break;
        }
            
        case TOF_INIT_READY_DELAY:
            if (millis() - tofInitStartTime >= 50) {
                Serial.print("Initializing VL53L0X... ");
                tofInitStartTime = millis();
                tofInitState = TOF_INIT_CALLING_INIT;
            }
            break;
            
        case TOF_INIT_CALLING_INIT: {
            // Call updateOTA() right before the blocking init() call to maximize OTA availability
            updateOTA();
            // This is the blocking call - but we've serviced OTA right before it
            // Note: We can't interrupt this call, but OTA was serviced immediately before it
            bool initResult = tofSensor.init();
            // Call updateOTA() immediately after init() completes
            updateOTA();
            unsigned long initDuration = millis() - tofInitStartTime;
            
            if (!initResult) {
                Serial.println("FAILED!");
                Serial.print("Init took ");
                Serial.print(initDuration);
                Serial.println("ms");
                Serial.println("Check wiring: SDA->GPIO10, SCL->GPIO11, VIN->3.3V, GND->GND");
                tofInitState = TOF_INIT_FAILED;
                tofInitialized = false;
            } else {
                Serial.print("OK (");
                Serial.print(initDuration);
                Serial.println("ms)");
                tofInitState = TOF_INIT_CONFIGURING;
            }
            break;
        }
            
        case TOF_INIT_CONFIGURING:
            tofSensor.setTimeout(500);
            tofSensor.setMeasurementTimingBudget(33000);
            tofInitialized = true;
            tofInitState = TOF_INIT_COMPLETE;
            Serial.println("ToF sensor initialized successfully!");
            Serial.println("Place an object 5-200cm in front of sensor and type 'ToF' to test");
            break;
            
        case TOF_INIT_COMPLETE:
        case TOF_INIT_FAILED:
            // Already done, do nothing
            break;
    }
}

void initializeToF() {
    // Start non-blocking initialization state machine
    // Actual initialization happens incrementally in updateToFInit() called from loop()
    if (tofInitState == TOF_INIT_NOT_STARTED) {
        tofInitState = TOF_INIT_NOT_STARTED; // Will start on first call to updateToFInit()
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

