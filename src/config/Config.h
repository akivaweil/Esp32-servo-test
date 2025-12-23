#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "Everwood"
#define WIFI_PASSWORD "Everwood-Staff"

// Ultrasonic Sensor Pins (HC-SR04)
#define ULTRASONIC_TRIG_PIN 16  // Trigger pin (OUTPUT)
#define ULTRASONIC_ECHO_PIN 17  // Echo pin (INPUT)

// Time of Flight Sensor Pins (VL53L0X - I2C)
#define TOF_SDA_PIN 10  // I2C Data pin
#define TOF_SCL_PIN 11  // I2C Clock pin
#define TOF_XSHUT_PIN 1 // XSHUT (shutdown) pin - LOW = disabled, HIGH = enabled

#endif

