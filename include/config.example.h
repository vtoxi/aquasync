#pragma once

// Copy this file to include/config.h and fill in your own network/tank
// values. config.h is gitignored on purpose -- it holds your WiFi
// credentials and MQTT broker address, neither of which belong in a
// public repo.

// ---- WiFi ------------------------------------------------------------
#define WIFI_SSID       "your-wifi-ssid"
#define WIFI_PASSWORD   "your-wifi-password"

// ---- MQTT (broker running on the same box as your OpenHAB instance) --
#define MQTT_HOST       "192.168.1.10"
#define MQTT_PORT       1883
#define MQTT_USER       ""              // leave blank if the broker is open
#define MQTT_PASSWORD   ""
#define MQTT_CLIENT_ID  "aquasync-01"
#define MQTT_TOPIC_PREFIX "aquasync"    // publishes under aquasync/level, aquasync/motor, ...

// ---- OTA (upload a new firmware .bin at http://OTA_HOSTNAME.local/) ----
// Plain HTTP Basic Auth -- fine on a trusted home LAN, don't expose this
// port to the internet. Change the password; the OTA endpoint can flash
// arbitrary firmware to the board.
#define OTA_USERNAME    "admin"
#define OTA_PASSWORD    "change-me"
#define OTA_HOSTNAME    "aquasync"      // reachable at http://aquasync.local/

// ---- Tank geometry -----------------------------------------------------
// Sensor is mounted at the top of the tank, facing straight down.
#define TANK_HEIGHT_CM       150   // distance from sensor face to tank bottom
#define SENSOR_DEADBAND_CM   5     // distance reading at 100% full (mounting offset)

// ---- Control thresholds (hysteresis, in percent full) ------------------
#define LOW_THRESHOLD_PCT    20    // motor turns ON at or below this level
#define HIGH_THRESHOLD_PCT   90    // motor turns OFF at or above this level

// ---- Safety ------------------------------------------------------------
#define MAX_RUNTIME_MINUTES  20    // if the motor runs this long without reaching
                                    // HIGH_THRESHOLD_PCT, assume a dry run / stuck
                                    // sensor and cut power rather than keep pumping
#define SENSOR_FAULT_READS   5     // consecutive bad ultrasonic reads before
                                    // the sensor is flagged faulty
