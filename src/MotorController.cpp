#include "MotorController.h"

MotorController::MotorController(uint8_t relayPin, uint8_t lowPct, uint8_t highPct, uint16_t maxRuntimeMinutes)
    : relayPin_(relayPin), lowPct_(lowPct), highPct_(highPct),
      maxRuntimeMs_(static_cast<uint32_t>(maxRuntimeMinutes) * 60UL * 1000UL) {}

void MotorController::begin() {
    pinMode(relayPin_, OUTPUT);
    setRelay(false);
}

void MotorController::setRelay(bool on) {
    running_ = on;
    digitalWrite(relayPin_, on ? HIGH : LOW);
    if (on) {
        runStartedMs_ = millis();
    }
}

void MotorController::update(uint8_t levelPercent, bool sensorFaulted) {
    if (safetyTripped_) {
        setRelay(false);
        return;
    }

    // A faulted sensor means we can't trust the level reading at all --
    // never run the pump blind. Force it off and wait for a human.
    if (sensorFaulted) {
        setRelay(false);
        return;
    }

    if (mode_ == MotorMode::MANUAL_OFF) {
        setRelay(false);
        return;
    }
    if (mode_ == MotorMode::MANUAL_ON) {
        if (!running_) setRelay(true);
    } else { // AUTO: classic hysteresis band
        if (!running_ && levelPercent <= lowPct_) {
            setRelay(true);
        } else if (running_ && levelPercent >= highPct_) {
            setRelay(false);
        }
    }

    if (running_ && (millis() - runStartedMs_ > maxRuntimeMs_)) {
        // Ran the whole safety window without reaching the high mark.
        // Likely a dry well, a stuck check valve, or a bad reading we
        // didn't catch -- stop and require a manual reset.
        setRelay(false);
        safetyTripped_ = true;
    }
}
