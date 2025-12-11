#include <Arduino.h>
#include "Paint_Motor_Controller.h"
#include "Ultrasonic_Sensor.h"

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

const unsigned long ULTRASONIC_READ_INTERVAL = 100; // Read every 100ms (10 readings/second)
unsigned long lastUltrasonicRead = 0;
bool ultrasonicActive = false; // Control flag for ultrasonic readings

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
    Serial.println("Type 'Ultra' to start readings, 'stop' to stop");
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
            Serial.println("Ultrasonic sensing started!");
        } else if (command == "stop") {
            ultrasonicActive = false;
            Serial.println("Ultrasonic sensing stopped!");
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
        if (currentTime - lastUltrasonicRead >= ULTRASONIC_READ_INTERVAL) {
            float distanceCm = readDistance();
            float distanceInches = readDistanceInches();
            
            Serial.print("Distance: ");
            Serial.print(distanceCm);
            Serial.print(" cm (");
            Serial.print(distanceInches);
            Serial.println(" inches)");
            
            lastUltrasonicRead = currentTime;
        }
    }
    
    delay(10);
}

