#include "MqttHandler.h"
#include "Version.h"

namespace {
// PubSubClient's callback is a plain function pointer, so it can't bind
// directly to a member function. One handler instance is all this
// project ever needs, so a single static pointer to bridge the two is
// simpler than pulling in std::function for a handler with no args worth
// capturing beyond "this".
MqttHandler* activeInstance = nullptr;

void staticCallback(char* topic, uint8_t* payload, unsigned int length) {
    if (activeInstance) {
        activeInstance->onMessage(topic, payload, length);
    }
}
}

MqttHandler::MqttHandler(WiFiClient& netClient, const char* host, uint16_t port,
                          const char* clientId, const char* user, const char* password,
                          const char* topicPrefix)
    : client_(netClient), host_(host), port_(port), clientId_(clientId),
      user_(user), password_(password), prefix_(topicPrefix) {
    activeInstance = this;
}

void MqttHandler::begin() {
    client_.setServer(host_.c_str(), port_);
    client_.setCallback(staticCallback);
}

void MqttHandler::reconnect() {
    if (client_.connected()) return;

    bool ok;
    if (user_.length() > 0) {
        ok = client_.connect(clientId_.c_str(), user_.c_str(), password_.c_str());
    } else {
        ok = client_.connect(clientId_.c_str());
    }
    if (ok) {
        client_.subscribe((prefix_ + "/cmd/mode").c_str());
        client_.subscribe((prefix_ + "/cmd/reset").c_str());
        // Doesn't change without a flash, so publishing once per connect
        // (rather than every publishState()) is enough.
        client_.publish((prefix_ + "/version").c_str(), FIRMWARE_VERSION, true);
    }
    // Deliberately not blocking/retrying with delay() here -- main.cpp's
    // loop keeps running (sensor reads, safety checks) even while MQTT
    // is down. Losing the broker should never stop the pump logic.
}

void MqttHandler::loop() {
    if (!client_.connected()) {
        reconnect();
    }
    client_.loop();
}

void MqttHandler::publishState(uint8_t levelPercent, bool motorRunning, MotorMode mode,
                                bool sensorFaulted, bool safetyTripped) {
    if (!client_.connected()) return;

    client_.publish((prefix_ + "/level").c_str(), String(levelPercent).c_str(), true);
    client_.publish((prefix_ + "/motor").c_str(), motorRunning ? "ON" : "OFF", true);

    const char* modeStr = mode == MotorMode::AUTO ? "AUTO"
                         : mode == MotorMode::MANUAL_ON ? "MANUAL_ON" : "MANUAL_OFF";
    client_.publish((prefix_ + "/mode").c_str(), modeStr, true);

    const char* faultStr = safetyTripped ? "SAFETY_TRIP" : sensorFaulted ? "SENSOR" : "OK";
    client_.publish((prefix_ + "/fault").c_str(), faultStr, true);
}

void MqttHandler::onMessage(char* topic, uint8_t* payload, unsigned int length) {
    String topicStr(topic);
    String value;
    for (unsigned int i = 0; i < length; i++) value += static_cast<char>(payload[i]);
    value.trim();
    value.toUpperCase();

    if (topicStr == prefix_ + "/cmd/mode") {
        if (value == "AUTO") {
            pendingMode_ = MotorMode::AUTO;
            pendingModeCommand_ = true;
        } else if (value == "ON") {
            pendingMode_ = MotorMode::MANUAL_ON;
            pendingModeCommand_ = true;
        } else if (value == "OFF") {
            pendingMode_ = MotorMode::MANUAL_OFF;
            pendingModeCommand_ = true;
        }
    } else if (topicStr == prefix_ + "/cmd/reset") {
        pendingReset_ = true;
    }
}

MotorMode MqttHandler::consumeModeCommand() {
    pendingModeCommand_ = false;
    return pendingMode_;
}

bool MqttHandler::consumeResetCommand() {
    bool r = pendingReset_;
    pendingReset_ = false;
    return r;
}
