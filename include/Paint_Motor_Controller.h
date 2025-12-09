#ifndef PAINT_MOTOR_CONTROLLER_H
#define PAINT_MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <FastAccelStepper.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ STEPPER CONFIGURATION                                              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Stepper Motor Pins
#define STEPPER_STEP_PIN 14
#define STEPPER_DIR_PIN 13 
#define STEPPER_ENABLE_PIN 10

// Stepper Motor Parameters
const float STEPPER_SPEED = 1000.0;
const float STEPPER_ACCEL = 1000.0;

// Function Declarations
void initializeStepper();
void moveStepper(long steps);
void stopStepper();
bool isStepperRunning();
long getStepperPosition();
void disableStepper();

#endif

