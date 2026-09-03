#pragma once
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "MotorController.h"

// Publishes tank state and takes commands over MQTT so the tank shows up
// as a normal set of items in OpenHAB (via the MQTT binding) instead of
// needing a bespoke integration. Topics, all under MQTT_TOPIC_PREFIX:
//
//   <prefix>/level        (pub, retained)  0-100
//   <prefix>/motor        (pub, retained)  "ON" | "OFF"
//   <prefix>/mode         (pub, retained)  "AUTO" | "MANUAL_ON" | "MANUAL_OFF"
//   <prefix>/fault        (pub, retained)  "OK" | "SENSOR" | "SAFETY_TRIP"
//   <prefix>/version      (pub, retained)  firmware version (see Version.h) --
//                                           lets OpenHAB confirm an OTA update took
//   <prefix>/cmd/mode     (sub)            set mode from OpenHAB
//   <prefix>/cmd/reset    (sub)            any payload clears a safety trip
class MqttHandler {
public:
    MqttHandler(WiFiClient& netClient, const char* host, uint16_t port,
                const char* clientId, const char* user, const char* password,
                const char* topicPrefix);

    void begin();
    void loop(); // call every iteration; reconnects as needed

    // Public only so the free function that bridges PubSubClient's plain
    // function-pointer callback can reach it -- not meant to be called
    // from outside that bridge.
    void onMessage(char* topic, uint8_t* payload, unsigned int length);

    void publishState(uint8_t levelPercent, bool motorRunning, MotorMode mode,
                       bool sensorFaulted, bool safetyTripped);

    // Set by the MQTT callback when a command comes in; main.cpp polls
    // these once per loop and clears them after acting.
    bool hasModeCommand() const { return pendingModeCommand_; }
    MotorMode consumeModeCommand();
    bool consumeResetCommand();

private:
    PubSubClient client_;
    String host_;
    uint16_t port_;
    String clientId_;
    String user_;
    String password_;
    String prefix_;

    bool pendingModeCommand_ = false;
    MotorMode pendingMode_ = MotorMode::AUTO;
    bool pendingReset_ = false;

    void reconnect();
};
