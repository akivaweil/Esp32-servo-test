#include <Arduino.h>
#include "Paint_Motor_Controller.h"
#include "Ultrasonic_Sensor.h"
#include "TimeOfFlight_Sensor.h"

// Forward declarations
void initializeOTA();
void updateOTA();

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ MOVEMENT CONFIGURATION                                             ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

const long MIN_STEPS = 3000;
const long MAX_STEPS = 15000;
const unsigned long MOVEMENT_DELAY = 1000; // Delay between movements (ms)
const unsigned long SEQUENCE_DURATION = 10000; // Run for 10 seconds (ms)

bool movementActive = false;
unsigned long lastMovementTime = 0;
unsigned long sequenceStartTime = 0;
bool sequenceComplete = false;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 ULTRASONIC SENSOR CONFIGURATION                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

const unsigned long ULTRASONIC_READ_INTERVAL = 50; // Sense every 50ms
const unsigned long ULTRASONIC_PRINT_INTERVAL = 500; // Print averaged result every 500ms
unsigned long lastUltrasonicRead = 0;
unsigned long lastUltrasonicPrint = 0;
bool ultrasonicActive = false; // Control flag for ultrasonic readings
float distanceSum = 0.0; // Accumulator for averaging
unsigned int readingCount = 0; // Count of readings for averaging

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 TIME OF FLIGHT SENSOR CONFIGURATION                                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

const unsigned long TOF_READ_INTERVAL = 50; // Sense every 50ms
const unsigned long TOF_PRINT_INTERVAL = 500; // Print averaged result every 500ms
unsigned long lastToFRead = 0;
unsigned long lastToFPrint = 0;
bool tofActive = false; // Control flag for ToF readings
float tofDistanceSum = 0.0; // Accumulator for averaging
unsigned int tofReadingCount = 0; // Count of readings for averaging

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🚀 SETUP                                                              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize OTA
    initializeOTA();
    
    // Initialize stepper motor
    initializeStepper();
    sequenceStartTime = millis();
    
    // Initialize ultrasonic sensor
    initializeUltrasonic();
    Serial.println("Ultrasonic sensor initialized!");
    
    // Initialize ToF sensor
    initializeToF();
    
    Serial.println("Type 'Ultra' to start ultrasonic readings");
    Serial.println("Type 'ToF' to start ToF sensor readings");
    Serial.println("Type 'stop' to stop readings");
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 LOOP                                                               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void loop() {
    // Update OTA
    updateOTA();
    
    //╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
    //║ 📡 SERIAL COMMAND PARSING                                            ║
    //╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
    
    // Check for serial commands
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Remove whitespace
        command.toLowerCase(); // Convert to lowercase for case-insensitive matching
        
        if (command == "ultra") {
            ultrasonicActive = true;
            tofActive = false; // Stop ToF when starting ultrasonic
            // Reset accumulators when starting
            distanceSum = 0.0;
            readingCount = 0;
            tofDistanceSum = 0.0;
            tofReadingCount = 0;
            Serial.println("Ultrasonic sensing started!");
        } else if (command == "tof") {
            tofActive = true;
            ultrasonicActive = false; // Stop ultrasonic when starting ToF
            // Reset accumulators when starting
            tofDistanceSum = 0.0;
            tofReadingCount = 0;
            distanceSum = 0.0;
            readingCount = 0;
            Serial.println("ToF sensor sensing started!");
        } else if (command == "stop") {
            ultrasonicActive = false;
            tofActive = false;
            // Reset accumulators when stopping
            distanceSum = 0.0;
            readingCount = 0;
            tofDistanceSum = 0.0;
            tofReadingCount = 0;
            Serial.println("All sensing stopped!");
        }
    }
    
    // Handle movement sequence
    if (!sequenceComplete) {
        unsigned long currentTime = millis();
        
        // Check if 10 seconds have passed
        if (currentTime - sequenceStartTime >= SEQUENCE_DURATION) {
            // Stop motor and disable
            stopStepper();
            disableStepper();
            sequenceComplete = true;
        } else {
            // Check if movement is complete
            if (movementActive && !isStepperRunning()) {
                movementActive = false;
                lastMovementTime = currentTime;
            }
            
            // Start new random movement after delay
            if (!movementActive && (currentTime - lastMovementTime >= MOVEMENT_DELAY)) {
                // Generate random steps between MIN_STEPS and MAX_STEPS
                long randomSteps = random(MIN_STEPS, MAX_STEPS + 1);
                
                // Random direction (positive or negative)
                if (random(0, 2) == 0) {
                    randomSteps = -randomSteps;
                }
                
                // Start movement
                moveStepper(randomSteps);
                movementActive = true;
            }
        }
    }
    
    // Read and display ultrasonic sensor distance (only when active)
    if (ultrasonicActive) {
        unsigned long currentTime = millis();
        
        // Sense every 50ms and accumulate readings
        if (currentTime - lastUltrasonicRead >= ULTRASONIC_READ_INTERVAL) {
            float distanceCm = readDistance();
            
            // Only add valid readings (non-zero)
            if (distanceCm > 0.0) {
                distanceSum += distanceCm;
                readingCount++;
            }
            
            lastUltrasonicRead = currentTime;
        }
        
        // Print averaged result every 500ms
        if (currentTime - lastUltrasonicPrint >= ULTRASONIC_PRINT_INTERVAL) {
            if (readingCount > 0) {
                float averageDistanceCm = distanceSum / readingCount;
                
                Serial.print("Ultrasonic Distance: ");
                Serial.print(averageDistanceCm);
                Serial.println(" cm");
                
                // Reset accumulator
                distanceSum = 0.0;
                readingCount = 0;
            }
            
            lastUltrasonicPrint = currentTime;
        }
    }
    
    //╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
    //║ 📡 TIME OF FLIGHT SENSOR TEST                                        ║
    //╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
    
    // Read and display ToF sensor distance (only when active)
    if (tofActive) {
        unsigned long currentTime = millis();
        
        // Sense every 50ms and accumulate readings
        if (currentTime - lastToFRead >= TOF_READ_INTERVAL) {
            float distanceCm = readToFDistanceCm();
            
            // Only add valid readings (non-zero)
            if (distanceCm > 0.0) {
                tofDistanceSum += distanceCm;
                tofReadingCount++;
            }
            
            lastToFRead = currentTime;
        }
        
        // Print averaged result every 500ms
        if (currentTime - lastToFPrint >= TOF_PRINT_INTERVAL) {
            if (tofReadingCount > 0) {
                float averageDistanceCm = tofDistanceSum / tofReadingCount;
                
                Serial.print("ToF Distance: ");
                Serial.print(averageDistanceCm);
                Serial.println(" cm");
            } else {
                // Show raw value for debugging
                float rawMm = readToFRawDistance();
                Serial.print("ToF: No valid readings - Raw value: ");
                Serial.print(rawMm);
                Serial.print(" mm (");
                if (rawMm < 30) {
                    Serial.println("too close, need >30mm)");
                } else if (rawMm > 2000) {
                    Serial.println("too far, max 2000mm)");
                } else {
                    Serial.println("check sensor)");
                }
            }
            
            // Reset accumulator
            tofDistanceSum = 0.0;
            tofReadingCount = 0;
            
            lastToFPrint = currentTime;
        }
    }
    
    delay(10);
}

