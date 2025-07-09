#include <Arduino.h>
#include "ServoControl.h"
#include "OTA/OTA_Upload.h"

// Pin definitions
const int SERVO_PIN = 15;

// Hardware objects
ServoControl myServo;

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to connect
  Serial.println("Booting...");

  // Initialize OTA
  Serial.println("Setting up OTA...");
  setupOTA();
  Serial.println("OTA setup complete.");

  // Initialize servo
  Serial.println("Initializing servo...");
  myServo.init(SERVO_PIN);
  Serial.println("Servo initialized.");
}

void loop() {
  // Handle OTA
  handleOTA();

  // Move servo
  myServo.write(0);
  delay(3000);

  myServo.write(90);
  delay(3000);

  myServo.write(180);
  delay(3000);

  myServo.write(90);
  delay(3000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

