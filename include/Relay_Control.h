#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <Arduino.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ RELAY CONFIGURATION                                                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

#include "Config/Pins_Definitions.h"

// Function Declarations
void initializeRelay();
void setRelay(bool state);
bool getRelayState();

#endif

