#include "DisplayUI.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pins.h"

namespace {
constexpr uint8_t kScreenWidth = 128;
constexpr uint8_t kScreenHeight = 64;
constexpr uint8_t kOledAddr = 0x3C;
Adafruit_SSD1306 display(kScreenWidth, kScreenHeight, &Wire, -1);
}

bool DisplayUI::begin() {
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, kOledAddr)) {
        return false; // OLED is a nice-to-have; firmware runs fine without it
    }
    display.clearDisplay();
    display.display();
    return true;
}

void DisplayUI::render(uint8_t levelPercent, bool motorRunning, MotorMode mode,
                        bool sensorFaulted, bool safetyTripped, bool wifiConnected) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.printf("%3d%%", levelPercent);

    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print(motorRunning ? "MOTOR: ON" : "MOTOR: OFF");

    display.setCursor(0, 32);
    switch (mode) {
        case MotorMode::AUTO:       display.print("MODE: AUTO"); break;
        case MotorMode::MANUAL_ON:  display.print("MODE: MANUAL ON"); break;
        case MotorMode::MANUAL_OFF: display.print("MODE: MANUAL OFF"); break;
    }

    display.setCursor(0, 44);
    if (safetyTripped) {
        display.print("!! SAFETY TRIP !!");
    } else if (sensorFaulted) {
        display.print("!! SENSOR FAULT !!");
    } else {
        display.print(wifiConnected ? "WiFi: connected" : "WiFi: offline");
    }

    display.display();
}
