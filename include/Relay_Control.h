#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ RELAY CONFIGURATION                                                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

#include "Config/Pins_Definitions.h"

// SSR Logic Configuration
// Set to true if SSR is active LOW (many SSRs use inverted logic)
#define SSR_ACTIVE_LOW true

// Function Declarations
void initializeRelay();
void setRelay(bool state);
bool getRelayState();
void forceRelayOff(); // Force relay off regardless of state

#endif

