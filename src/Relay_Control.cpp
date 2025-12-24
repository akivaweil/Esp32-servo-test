#include "Relay_Control.h"

#ifdef ESP32
#include <driver/gpio.h>
#endif

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔌 RELAY STATE                                                         ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

bool relayState = false;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ INITIALIZE RELAY                                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void initializeRelay() {
    pinMode(RELAY_PIN, OUTPUT);
    // Set drive strength to maximum for better SSR control
    #ifdef ESP32
    gpio_set_drive_capability((gpio_num_t)RELAY_PIN, GPIO_DRIVE_CAP_3);
    #endif
    
    // Force relay off immediately - set state multiple times to ensure it sticks
    if (SSR_ACTIVE_LOW) {
        digitalWrite(RELAY_PIN, HIGH); // Start with relay off (SSR active LOW)
        delayMicroseconds(100);
        digitalWrite(RELAY_PIN, HIGH); // Repeat to ensure state
    } else {
        digitalWrite(RELAY_PIN, LOW); // Start with relay off (SSR active HIGH)
        delayMicroseconds(100);
        digitalWrite(RELAY_PIN, LOW); // Repeat to ensure state
    }
    relayState = false;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ SET RELAY STATE                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void setRelay(bool state) {
    relayState = state;
    // Invert logic for active LOW SSR
    // Write multiple times to ensure state change registers
    if (SSR_ACTIVE_LOW) {
        digitalWrite(RELAY_PIN, state ? LOW : HIGH); // LOW = on, HIGH = off
        delayMicroseconds(50);
        digitalWrite(RELAY_PIN, state ? LOW : HIGH); // Repeat to ensure state
    } else {
        digitalWrite(RELAY_PIN, state ? HIGH : LOW); // HIGH = on, LOW = off
        delayMicroseconds(50);
        digitalWrite(RELAY_PIN, state ? HIGH : LOW); // Repeat to ensure state
    }
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ GET RELAY STATE                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

bool getRelayState() {
    return relayState;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ FORCE RELAY OFF                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void forceRelayOff() {
    relayState = false;
    // Force pin to OFF state multiple times with delays
    if (SSR_ACTIVE_LOW) {
        for (int i = 0; i < 5; i++) {
            digitalWrite(RELAY_PIN, HIGH); // HIGH = off for active LOW SSR
            delayMicroseconds(200);
        }
    } else {
        for (int i = 0; i < 5; i++) {
            digitalWrite(RELAY_PIN, LOW); // LOW = off for active HIGH SSR
            delayMicroseconds(200);
        }
    }
}

