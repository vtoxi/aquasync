#pragma once
#include <Arduino.h>
#include "MotorController.h"

// Thin wrapper around the 0.96" SSD1306 status screen. Kept deliberately
// dumb -- it just renders whatever state it's handed each loop, no
// business logic lives here.
class DisplayUI {
public:
    bool begin();
    void render(uint8_t levelPercent, bool motorRunning, MotorMode mode,
                bool sensorFaulted, bool safetyTripped, bool wifiConnected);
};
