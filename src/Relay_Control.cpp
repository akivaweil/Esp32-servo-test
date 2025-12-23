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
    digitalWrite(RELAY_PIN, LOW); // Start with relay off (Active HIGH)
    relayState = false;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ SET RELAY STATE                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void setRelay(bool state) {
    relayState = state;
    digitalWrite(RELAY_PIN, state ? HIGH : LOW);
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ GET RELAY STATE                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

bool getRelayState() {
    return relayState;
}

