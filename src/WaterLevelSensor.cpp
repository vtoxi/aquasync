#include "WaterLevelSensor.h"
#include <algorithm>

namespace {
constexpr uint8_t kSamplesPerRead = 5;
constexpr uint32_t kEchoTimeoutUs = 30000;   // ~5m round trip, well past any tank we'd use this on
constexpr float kSoundCmPerUs = 0.0343f / 2; // speed of sound / 2 (there and back)
}

WaterLevelSensor::WaterLevelSensor(uint8_t trigPin, uint8_t echoPin, uint16_t tankHeightCm, uint16_t deadbandCm)
    : trigPin_(trigPin), echoPin_(echoPin), tankHeightCm_(tankHeightCm), deadbandCm_(deadbandCm) {}

void WaterLevelSensor::begin() {
    pinMode(trigPin_, OUTPUT);
    pinMode(echoPin_, INPUT);
    digitalWrite(trigPin_, LOW);
}

int16_t WaterLevelSensor::sampleOnceCm() {
    digitalWrite(trigPin_, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_, LOW);

    unsigned long durationUs = pulseIn(echoPin_, HIGH, kEchoTimeoutUs);
    if (durationUs == 0) {
        return -1; // no echo before timeout
    }
    return static_cast<int16_t>(durationUs * kSoundCmPerUs);
}

uint8_t WaterLevelSensor::readPercent() {
    int16_t samples[kSamplesPerRead];
    uint8_t validCount = 0;

    for (uint8_t i = 0; i < kSamplesPerRead; i++) {
        int16_t cm = sampleOnceCm();
        if (cm > 0 && cm <= tankHeightCm_) {
            samples[validCount++] = cm;
        }
        delay(30); // let ripples settle between pings
    }

    if (validCount < 3) {
        // too many bad reads in this burst to trust a median
        consecutiveBadReads_++;
        return 0;
    }
    consecutiveBadReads_ = 0;

    std::sort(samples, samples + validCount);
    int16_t distanceCm = samples[validCount / 2]; // median

    // distanceCm is measured from the sensor down to the water surface;
    // closer to the sensor (accounting for the mounting deadband) means fuller.
    int16_t usableHeight = tankHeightCm_ - deadbandCm_;
    int16_t fromFull = distanceCm - deadbandCm_;
    if (fromFull < 0) fromFull = 0;
    if (fromFull > usableHeight) fromFull = usableHeight;

    float percent = 100.0f * (1.0f - (static_cast<float>(fromFull) / usableHeight));
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return static_cast<uint8_t>(percent + 0.5f);
}
