#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "config.h"
#include "pins.h"
#include "WaterLevelSensor.h"
#include "MotorController.h"
#include "DisplayUI.h"
#include "MqttHandler.h"

namespace {
constexpr uint32_t kLoopIntervalMs = 2000;    // full sensor-read + control cycle
constexpr uint32_t kWifiRetryMs = 15000;

WaterLevelSensor sensor(PIN_ULTRASONIC_TRIG, PIN_ULTRASONIC_ECHO, TANK_HEIGHT_CM, SENSOR_DEADBAND_CM);
MotorController motor(PIN_RELAY, LOW_THRESHOLD_PCT, HIGH_THRESHOLD_PCT, MAX_RUNTIME_MINUTES);
DisplayUI display;
WiFiClient netClient;
MqttHandler mqtt(netClient, MQTT_HOST, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_PREFIX);

uint32_t lastWifiAttemptMs = 0;
bool lastButtonState = HIGH;

void connectWifi() {
    if (WiFi.status() == WL_CONNECTED) return;
    if (millis() - lastWifiAttemptMs < kWifiRetryMs) return;

    lastWifiAttemptMs = millis();
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // Non-blocking: connection state is just polled each loop. The
    // control logic below never depends on WiFi being up.
}

void handleManualButton() {
    bool state = digitalRead(PIN_MANUAL_BUTTON);
    if (state == LOW && lastButtonState == HIGH) {
        // Single tap cycles AUTO -> MANUAL_ON -> MANUAL_OFF -> AUTO,
        // so the tank is still operable by hand if WiFi/MQTT is down.
        switch (motor.mode()) {
            case MotorMode::AUTO:       motor.setMode(MotorMode::MANUAL_ON); break;
            case MotorMode::MANUAL_ON:  motor.setMode(MotorMode::MANUAL_OFF); break;
            case MotorMode::MANUAL_OFF: motor.setMode(MotorMode::AUTO); break;
        }
    }
    lastButtonState = state;
}
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_MANUAL_BUTTON, INPUT_PULLUP);
    pinMode(PIN_FAULT_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    sensor.begin();
    sensor.setFaultThreshold(SENSOR_FAULT_READS);
    motor.begin();
    mqtt.begin();

    if (!display.begin()) {
        Serial.println("OLED not found -- continuing without local display");
    }

    connectWifi();
}

void loop() {
    static uint32_t lastCycleMs = 0;

    connectWifi();
    mqtt.loop();
    handleManualButton();

    if (mqtt.hasModeCommand()) {
        motor.setMode(mqtt.consumeModeCommand());
    }
    if (mqtt.consumeResetCommand()) {
        motor.resetSafetyTrip();
    }

    if (millis() - lastCycleMs < kLoopIntervalMs) {
        return;
    }
    lastCycleMs = millis();

    uint8_t levelPercent = sensor.readPercent();
    bool sensorFaulted = sensor.isFaulted();

    motor.update(levelPercent, sensorFaulted);

    bool alarm = sensorFaulted || motor.isSafetyTripped();
    digitalWrite(PIN_FAULT_LED, alarm ? HIGH : LOW);
    digitalWrite(PIN_BUZZER, alarm ? HIGH : LOW);

    display.render(levelPercent, motor.isRunning(), motor.mode(),
                    sensorFaulted, motor.isSafetyTripped(), WiFi.status() == WL_CONNECTED);

    mqtt.publishState(levelPercent, motor.isRunning(), motor.mode(),
                       sensorFaulted, motor.isSafetyTripped());
}
