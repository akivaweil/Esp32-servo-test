#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"

//* ************************************************************************
//* ************************ OTA MANAGER ***********************************
//* ************************************************************************

// OTA status
bool otaInitialized = false;
int lastProgressPercent = -1;

// WiFi connection status
bool wifiConnected = false;
unsigned long lastWifiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 1000; // Check every 1 second

// Function to print IP address with formatting
void printIPAddress();

void initializeOTA() {
    if (otaInitialized) {
        return;
    }
    
    // Connect to WiFi (Non-blocking)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // We don't wait here anymore to allow faster startup
    // OTA will be initialized in updateOTA() once connected
}

void printIPAddress() {
    Serial.println("=== WIFI CONNECTED ===");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("======================");
}

void setupArduinoOTA() {
    // Configure ArduinoOTA
    ArduinoOTA.setHostname("PaintMachine");
    ArduinoOTA.setPassword("paint123");

    ArduinoOTA.onStart([]() {
        // Turn off status LED during update
        digitalWrite(STATUS_LED_PIN, LOW);
        lastProgressPercent = -1;
        Serial.println("OTA update started");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int percent = (progress * 100) / total;
        
        // Print at 0%, 25%, 50%, 75%, and 100%
        if (percent == 0 || percent == 25 || percent == 50 || percent == 75 || percent == 100) {
            if (percent != lastProgressPercent) {
                Serial.print("OTA progress: ");
                Serial.print(percent);
                Serial.println("%");
                lastProgressPercent = percent;
            }
        }
    });

    ArduinoOTA.onEnd([]() {
        // Turn on status LED when update complete
        digitalWrite(STATUS_LED_PIN, HIGH);
        Serial.println("OTA update complete");
    });

    ArduinoOTA.onError([](ota_error_t error) {
        // Blink error LED on update error
        digitalWrite(ERROR_LED_PIN, HIGH);
        Serial.print("OTA error: ");
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Authentication Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        } else {
            Serial.println(error);
        }
    });

    // Start OTA
    ArduinoOTA.begin();

    otaInitialized = true;
    Serial.println("OTA Initialized");
    printIPAddress();
}

void updateOTA() {
    // ALWAYS handle OTA first - this is non-blocking and must be called frequently
    if (otaInitialized) {
        ArduinoOTA.handle();
    }
    
    // Check WiFi connection status
    unsigned long currentTime = millis();
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
    
    // If WiFi just connected, initialize OTA
    if (!otaInitialized && currentlyConnected) {
        wifiConnected = true;
        printIPAddress();
        setupArduinoOTA();
    }
    
    // Periodic WiFi health check and reconnection
    if (currentTime - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
        if (!currentlyConnected) {
            // WiFi disconnected - try to reconnect
            if (wifiConnected) {
                wifiConnected = false;
                otaInitialized = false; // Reset OTA so it reinitializes when WiFi reconnects
            }
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        } else {
            // WiFi is connected
            if (!wifiConnected) {
                // Just reconnected
                wifiConnected = true;
                printIPAddress();
                if (!otaInitialized) {
                    setupArduinoOTA();
                }
            }
        }
        lastWifiCheck = currentTime;
    }
}

bool isOTAReady() {
    return otaInitialized && wifiConnected;
}

String getOTAIpAddress() {
    if (wifiConnected) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

bool isWiFiConnected() {
    return wifiConnected;
}
