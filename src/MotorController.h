#pragma once
#include <Arduino.h>

enum class MotorMode : uint8_t { AUTO, MANUAL_ON, MANUAL_OFF };

// Runs the actual pump-control state machine: hysteresis on/off around
// two thresholds, plus a runaway-runtime cutoff so a stuck float, a
// faulted sensor, or an empty well doesn't leave the pump running dry
// until someone notices.
class MotorController {
public:
    MotorController(uint8_t relayPin, uint8_t lowPct, uint8_t highPct, uint16_t maxRuntimeMinutes);

    void begin();

    // Call every loop with the latest level reading and whether the
    // sensor is currently trusted. Applies hysteresis + safety logic
    // and drives the relay pin directly.
    void update(uint8_t levelPercent, bool sensorFaulted);

    void setMode(MotorMode mode) { mode_ = mode; }
    MotorMode mode() const { return mode_; }
    bool isRunning() const { return running_; }
    bool isSafetyTripped() const { return safetyTripped_; }

    // Clears a safety trip after a fault has been physically checked --
    // intentionally not automatic, this isn't a state you want to
    // silently retry out of.
    void resetSafetyTrip() { safetyTripped_ = false; }

private:
    uint8_t relayPin_;
    uint8_t lowPct_;
    uint8_t highPct_;
    uint32_t maxRuntimeMs_;

    MotorMode mode_ = MotorMode::AUTO;
    bool running_ = false;
    bool safetyTripped_ = false;
    uint32_t runStartedMs_ = 0;

    void setRelay(bool on);
};
