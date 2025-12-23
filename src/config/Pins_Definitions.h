#ifndef PINS_DEFINITIONS_H
#define PINS_DEFINITIONS_H

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔌 PIN DEFINITIONS                                                   ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Ultrasonic Sensor Pins (HC-SR04)
#define ULTRASONIC_TRIG_PIN 16  // Trigger pin (OUTPUT)
#define ULTRASONIC_ECHO_PIN 17  // Echo pin (INPUT)

// Time of Flight Sensor Pins (VL53L0X - I2C)
#define TOF_SDA_PIN 10  // I2C Data pin
#define TOF_SCL_PIN 11  // I2C Clock pin
#define TOF_XSHUT_PIN 1 // XSHUT (shutdown) pin - LOW = disabled, HIGH = enabled

// Stepper Motor Pins
#define STEPPER_STEP_PIN 14
#define STEPPER_DIR_PIN 13 
#define STEPPER_ENABLE_PIN 10

// LED Pins
#define STATUS_LED_PIN 2
#define ERROR_LED_PIN 3

// Button Pins
#define TEST_BUTTON_PIN 15  // Test button (INPUT, Active HIGH, Internal pull-down)

#endif

