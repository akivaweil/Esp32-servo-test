#include "Relay_Control.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔌 RELAY STATE                                                         ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

bool relayState = false;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ INITIALIZE RELAY                                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void initializeRelay() {
    pinMode(RELAY_PIN, OUTPUT);
    // Set initial state based on SSR logic type
    // For active LOW SSR: HIGH = off, LOW = on
    // For active HIGH SSR: LOW = off, HIGH = on
    if (SSR_ACTIVE_LOW) {
        digitalWrite(RELAY_PIN, HIGH); // Start with relay off (SSR active LOW)
    } else {
        digitalWrite(RELAY_PIN, LOW); // Start with relay off (SSR active HIGH)
    }
    relayState = false;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ SET RELAY STATE                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void setRelay(bool state) {
    relayState = state;
    // Invert logic for active LOW SSR
    if (SSR_ACTIVE_LOW) {
        digitalWrite(RELAY_PIN, state ? LOW : HIGH); // LOW = on, HIGH = off
    } else {
        digitalWrite(RELAY_PIN, state ? HIGH : LOW); // HIGH = on, LOW = off
    }
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ GET RELAY STATE                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

bool getRelayState() {
    return relayState;
}

