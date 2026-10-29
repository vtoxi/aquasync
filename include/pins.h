#pragma once

// Pin map for a generic ESP32 dev board. Works unchanged on ESP32-S3
// dev boards too -- these GPIOs aren't strapping pins on either variant.
// See docs/wiring.md for the full wiring diagram and the relay/motor
// safety notes.

#define PIN_ULTRASONIC_TRIG   5
#define PIN_ULTRASONIC_ECHO   18   // through a resistor divider -- sensor echo is 5V, ESP32 inputs are 3.3V

#define PIN_RELAY             26   // drives the pump contactor coil, not the pump itself
#define PIN_MANUAL_BUTTON     27   // INPUT_PULLUP, tap to toggle auto/manual mode
#define PIN_FAULT_LED         25
#define PIN_BUZZER            33

#define PIN_OLED_SDA          21   // SSD1306 128x64, I2C address 0x3C
#define PIN_OLED_SCL          22
