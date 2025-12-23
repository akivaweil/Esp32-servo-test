#ifndef CONFIG_H
#define CONFIG_H

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ CONFIGURATION                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// WiFi Configuration
#define WIFI_SSID "Everwood"
#define WIFI_PASSWORD "Everwood-Staff"

// Time of Flight Sensor Configuration
#define TOF_INIT_TIMEOUT_MS 2000 // Timeout for VL53L0X initialization (2 seconds)
#define TOF_TOTAL_INIT_TIMEOUT_MS 3000 // Total timeout for ToF sensor initialization (3 seconds)

#endif
