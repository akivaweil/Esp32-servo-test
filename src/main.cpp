#include <Arduino.h>
#include <FastAccelStepper.h>
#include "config/Pin_Definitions.h"

// Forward declarations
void initializeOTA();
void updateOTA();

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ STEPPER CONFIGURATION                                              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

const float STEPPER_SPEED = 10000.0;
const float STEPPER_ACCEL = 10000.0;
const long MIN_STEPS = 3000;
const long MAX_STEPS = 15000;

bool movementActive = false;
unsigned long lastMovementTime = 0;
const unsigned long MOVEMENT_DELAY = 1000; // Delay between movements (ms)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🚀 SETUP                                                              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize OTA
    initializeOTA();
    
    // Initialize stepper motor
    engine.init();
    stepper = engine.stepperConnectToPin(STEPPER_STEP_PIN);
    if (stepper) {
        stepper->setDirectionPin(STEPPER_DIR_PIN);
        stepper->setEnablePin(STEPPER_ENABLE_PIN);
        stepper->setAutoEnable(true);
        stepper->setSpeedInHz(STEPPER_SPEED);
        stepper->setAcceleration(STEPPER_ACCEL);
    }
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 LOOP                                                               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void loop() {
    // Update OTA
    updateOTA();
    
    // Handle stepper movement sequence
    if (stepper) {
        unsigned long currentTime = millis();
        
        // Check if movement is complete
        if (movementActive && stepper->isRunning() == false) {
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
            stepper->move(randomSteps);
            movementActive = true;
        }
    }
    
    delay(10);
}

